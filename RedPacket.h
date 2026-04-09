#ifndef REDPACKET_H
#define REDPACKET_H

#include <string>
#include <tchar.h>
using namespace std;

class RedPacket
{
public:
	enum EGrabStatus
	{
		GrabSuccess = 1,
		GrabEmpty = 0,
		GrabDuplicate = -1
	};

	typedef basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > text_string;

	RedPacket(double money = 0.0, int packetNum = 1, text_string owner = TEXT("Unknown"));
	~RedPacket();

	void setMoney(double money, int packetNum);
	double grab(text_string grabberName);
	double grab(text_string grabberName, int* outStatus);
	void show() const;

	text_string summary() const;
	// Return internal fixed array pointer; valid element count is grabbedCount().
	const text_string* records() const;
	bool canSetMoney() const;
	int grabbedCount() const;
	int totalCount() const;
	text_string bestLuckRecord() const;

private:
	// Copy is intentionally disabled because this class owns raw dynamic arrays.
	RedPacket(const RedPacket&);
	RedPacket& operator=(const RedPacket&);

	double total_money;
	int num;
	int grabbed;
	text_string name;
	text_string* arr;
	text_string* grabbed_names;
	int grabbed_name_count;

	double Round2(double value) const;
	bool HasGrabbed(const text_string& grabberName) const;
};

#endif
