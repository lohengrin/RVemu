#pragma once

#include "Device.h"

#include <map>
#include <algorithm>

class Bus {
public:
	Bus() : devices_sorted(false) {}
	virtual ~Bus() {}

	//! Device configuration
	bool addDevice(uint64_t baseaddr, Device* dev);
	Device* getDevice(uint64_t baseaddr);

	uint64_t load(uint64_t addr, uint8_t size) const;
	void store(uint64_t addr, uint8_t size, uint64_t value);

protected:
	struct DeviceEntry {
		DeviceEntry(uint64_t b, uint64_t s, Device* d) : base(b), end(b+s), device(d) {};
		uint64_t base;
		uint64_t end;
		Device* device;
		
		bool operator<(const DeviceEntry& other) const {
			return base < other.base;
		}
	};
	std::vector<DeviceEntry> myDevices;
	mutable bool devices_sorted;
	
	void ensureSorted() const;
};