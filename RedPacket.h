#ifndef REDPACKET_H
#define REDPACKET_H

#include <string>
#include <tchar.h>

using packet_text_string = std::basic_string<TCHAR>;

class RedPacket
{
public:
	enum EGrabStatus
	{
		GrabSuccess = 1,
		GrabEmpty = 0,
		GrabDuplicate = -1
	};

	RedPacket(double money = 0.0, int packetNum = 1, packet_text_string owner = TEXT("Unknown"));
	~RedPacket();

	void setMoney(double money, int packetNum);
	double grab(packet_text_string grabberName);
	double grab(packet_text_string grabberName, int* outStatus);
	void show() const;

	packet_text_string summary() const;
	// Return internal fixed array pointer; valid element count is grabbedCount().
	const packet_text_string* records() const;
	bool canSetMoney() const;
	int grabbedCount() const;
	int totalCount() const;
	packet_text_string bestLuckRecord() const;

private:
	// Copy is intentionally disabled because this class owns raw dynamic arrays.
	RedPacket(const RedPacket&);
	RedPacket& operator=(const RedPacket&);

	double total_money;
	int num;
	int grabbed;
	packet_text_string name;
	packet_text_string* arr;
	packet_text_string* grabbed_names;
	int grabbed_name_count;

	double Round2(double value) const;
	bool HasGrabbed(const packet_text_string& grabberName) const;
};

#endif
