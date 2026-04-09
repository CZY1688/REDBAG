#include "RedPacket.h"
#include <windows.h>

#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <random>
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <assert.h>
using namespace std;

// File-local defaults to keep implementation details private.
namespace
{
constexpr LPCTSTR DEFAULT_GRABBER_NAME = TEXT("AnonymousUser");
typedef basic_ostringstream<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > text_ostringstream;
}

double RedPacket::Round2(double value) const
{
	if (value >= 0.0) return static_cast<double>(static_cast<int>(value * 100.0 + 0.5)) / 100.0;
	return static_cast<double>(static_cast<int>(value * 100.0 - 0.5)) / 100.0;
}

RedPacket::RedPacket(double money, int packetNum, packet_text_string owner)
	: total_money(money), num(packetNum > 0 ? packetNum : 1), grabbed(0), name(owner), arr(0), grabbed_names(0), grabbed_name_count(0)
{
	if (name.empty()) name = TEXT("Unknown");
	if (total_money < 0.0) total_money = 0.0;
	arr = new packet_text_string[num];
	grabbed_names = new packet_text_string[num];
}

RedPacket::~RedPacket()
{
	delete[] arr;
	arr = 0;
	delete[] grabbed_names;
	grabbed_names = 0;
}

void RedPacket::setMoney(double money, int packetNum)
{
	if (grabbed > 0)
	{
		return;
	}
	if (packetNum <= 0 || money <= 0.0)
	{
		return;
	}
	total_money = Round2(money);
	num = packetNum;
	delete[] arr;
	arr = new packet_text_string[num];
	delete[] grabbed_names;
	grabbed_names = new packet_text_string[num];
	for (int i = 0; i < num; ++i)
	{
		arr[i].clear();
		grabbed_names[i].clear();
	}
	grabbed_name_count = 0;
}

double RedPacket::grab(packet_text_string grabberName)
{
	return grab(grabberName, 0);
}

bool RedPacket::HasGrabbed(const packet_text_string& grabberName) const
{
	for (int i = 0; i < grabbed_name_count; ++i)
	{
		if (grabbed_names[i] == grabberName) return true;
	}
	return false;
}

double RedPacket::grab(packet_text_string grabberName, int* outStatus)
{
	if (grabberName.empty()) grabberName = DEFAULT_GRABBER_NAME;
	if (HasGrabbed(grabberName))
	{
		if (outStatus) *outStatus = GrabDuplicate;
		return 0.0;
	}
	if (grabbed >= num)
	{
		if (outStatus) *outStatus = GrabEmpty;
		return 0.0;
	}

	double got = 0.0;
	int remainCount = num - grabbed;

	if (remainCount == 1)
	{
		got = Round2(total_money);
	}
	else
	{
		double avg = total_money / remainCount;
		double maxMoney = avg * 2.0;
		double minMoney = 0.01;
		double minRemain = static_cast<double>(remainCount - 1) * 0.01;
		double upperByRemain = total_money - minRemain;
		if (upperByRemain < minMoney) upperByRemain = minMoney;
		if (maxMoney > upperByRemain) maxMoney = upperByRemain;
		if (maxMoney < minMoney) maxMoney = minMoney;

		static thread_local mt19937 rng(random_device{}());
		int minFen = static_cast<int>(minMoney * 100.0 + 0.5);
		int maxFen = static_cast<int>(maxMoney * 100.0 + 0.5);
		if (maxFen < minFen) maxFen = minFen;
		uniform_int_distribution<int> distFen(minFen, maxFen);
		got = static_cast<double>(distFen(rng)) / 100.0;
		got = Round2(got);
		if (got < minMoney) got = minMoney;

		if (total_money - got < minRemain)
		{
			got = Round2(total_money - minRemain);
			if (got < minMoney) got = minMoney;
		}
	}

	total_money = Round2(total_money - got);
	if (total_money < 0.0) total_money = 0.0;

	text_ostringstream oss;
	oss << fixed << setprecision(2) << got;
	arr[grabbed] = grabberName + TEXT(":") + oss.str();
	assert(grabbed_name_count >= 0 && grabbed_name_count < num);
	grabbed_names[grabbed_name_count] = grabberName;
	++grabbed_name_count;
	++grabbed;
	if (outStatus) *outStatus = GrabSuccess;
	return got;
}

void RedPacket::show() const
{
	packet_text_string s = summary();
	OutputDebugString(s.c_str());
}

packet_text_string RedPacket::summary() const
{
	text_ostringstream oss;
	oss << TEXT("Owner: ") << name << TEXT("\n");
	oss << TEXT("Money left: ") << fixed << setprecision(2) << total_money << TEXT(" yuan\n");
	oss << TEXT("Total packets: ") << num << TEXT("\n");
	oss << TEXT("Grabbed count: ") << grabbed << TEXT("\n");
	for (int i = 0; i < grabbed; ++i)
	{
		oss << TEXT("  ") << i + 1 << TEXT(". ") << arr[i] << TEXT(" yuan\n");
	}
	packet_text_string best = bestLuckRecord();
	if (!best.empty()) oss << TEXT("Best luck: ") << best << TEXT(" yuan\n");
	return oss.str();
}

const packet_text_string* RedPacket::records() const
{
	return arr;
}

bool RedPacket::canSetMoney() const
{
	return grabbed == 0;
}

int RedPacket::grabbedCount() const
{
	return grabbed;
}

int RedPacket::totalCount() const
{
	return num;
}

packet_text_string RedPacket::bestLuckRecord() const
{
	double bestMoney = -1.0;
	packet_text_string bestName;
	for (int i = 0; i < grabbed; ++i)
	{
		size_t pos = arr[i].find(_T(':'));
		if (pos == packet_text_string::npos) continue;
		packet_text_string who = arr[i].substr(0, pos);
		packet_text_string moneyText = arr[i].substr(pos + 1);
		TCHAR* pEnd = 0;
		errno = 0;
		double money = _tcstod(moneyText.c_str(), &pEnd);
		if (pEnd == moneyText.c_str() || errno == ERANGE) continue;
		if (money > bestMoney)
		{
			bestMoney = money;
			bestName = who;
		}
	}
	if (bestMoney < 0.0 || bestName.empty()) return packet_text_string();
	text_ostringstream oss;
	oss << bestName << TEXT(":") << fixed << setprecision(2) << bestMoney;
	return oss.str();
}
