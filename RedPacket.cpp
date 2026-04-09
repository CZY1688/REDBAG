#include "RedPacket.h"
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
constexpr const TCHAR* DEFAULT_GRABBER_NAME = TEXT("AnonymousUser");
}

double RedPacket::Round2(double value) const
{
	if (value >= 0.0) return static_cast<double>(static_cast<int>(value * 100.0 + 0.5)) / 100.0;
	return static_cast<double>(static_cast<int>(value * 100.0 - 0.5)) / 100.0;
}

RedPacket::RedPacket(double money, int packetNum, tstring owner)
	: total_money(money), num(packetNum > 0 ? packetNum : 1), grabbed(0), name(owner), arr(0), grabbed_names(0), grabbed_name_count(0)
{
	if (name.empty()) name = TEXT("Unknown");
	if (total_money < 0.0) total_money = 0.0;
	arr = new tstring[num];
	grabbed_names = new tstring[num];
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
	arr = new tstring[num];
	delete[] grabbed_names;
	grabbed_names = new tstring[num];
	for (int i = 0; i < num; ++i)
	{
		arr[i].clear();
		grabbed_names[i].clear();
	}
	grabbed_name_count = 0;
}

double RedPacket::grab(tstring grabberName)
{
	return grab(grabberName, 0);
}

bool RedPacket::HasGrabbed(const tstring& grabberName) const
{
	for (int i = 0; i < grabbed_name_count; ++i)
	{
		if (grabbed_names[i] == grabberName) return true;
	}
	return false;
}

double RedPacket::grab(tstring grabberName, int* outStatus)
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

	std::basic_ostringstream<TCHAR> oss;
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
	tstring s = summary();
	OutputDebugString(s.c_str());
}

tstring RedPacket::summary() const
{
	std::basic_ostringstream<TCHAR> oss;
	oss << TEXT("Owner: ") << name << TEXT("\n");
	oss << TEXT("Money left: ") << fixed << setprecision(2) << total_money << TEXT(" yuan\n");
	oss << TEXT("Total packets: ") << num << TEXT("\n");
	oss << TEXT("Grabbed count: ") << grabbed << TEXT("\n");
	for (int i = 0; i < grabbed; ++i)
	{
		oss << TEXT("  ") << i + 1 << TEXT(". ") << arr[i] << TEXT(" yuan\n");
	}
	tstring best = bestLuckRecord();
	if (!best.empty()) oss << TEXT("Best luck: ") << best << TEXT(" yuan\n");
	return oss.str();
}

const tstring* RedPacket::records() const
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

tstring RedPacket::bestLuckRecord() const
{
	double bestMoney = -1.0;
	tstring bestName;
	for (int i = 0; i < grabbed; ++i)
	{
		size_t pos = arr[i].find(TEXT(":"));
		if (pos == tstring::npos) continue;
		tstring who = arr[i].substr(0, pos);
		tstring moneyText = arr[i].substr(pos + 1);
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
	if (bestMoney < 0.0 || bestName.empty()) return tstring();
	std::basic_ostringstream<TCHAR> oss;
	oss << bestName << TEXT(":") << fixed << setprecision(2) << bestMoney;
	return oss.str();
}
