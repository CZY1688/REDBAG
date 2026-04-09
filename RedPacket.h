#ifndef REDPACKET_H
#define REDPACKET_H

#include "BWindows.h"
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

	RedPacket(double money = 0.0, int packetNum = 1, tstring owner = TEXT("Unknown"));
	~RedPacket();

	void setMoney(double money, int packetNum);
	double grab(tstring grabberName);
	double grab(tstring grabberName, int* outStatus);
	void show() const;

	tstring summary() const;
	// Return internal fixed array pointer; valid element count is grabbedCount().
	const tstring* records() const;
	bool canSetMoney() const;
	int grabbedCount() const;
	int totalCount() const;
	tstring bestLuckRecord() const;

private:
	// Copy is intentionally disabled because this class owns raw dynamic arrays.
	RedPacket(const RedPacket&);
	RedPacket& operator=(const RedPacket&);

	double total_money;
	int num;
	int grabbed;
	tstring name;
	tstring* arr;
	tstring* grabbed_names;
	int grabbed_name_count;

	double Round2(double value) const;
	bool HasGrabbed(const tstring& grabberName) const;
};

#endif
