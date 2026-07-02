#include "Bus.h"
#include "Trap.h"

#include <iostream>
#include <algorithm>


//! Device configuration
bool Bus::addDevice(uint64_t baseaddr, Device* dev)
{
	if (!dev)
		return false;
	myDevices.push_back(DeviceEntry(baseaddr, dev->size(), dev));
	devices_sorted = false;
	return true;
}

Device* Bus::getDevice(uint64_t baseaddr)
{
	for (auto&& dev : myDevices)
	{
		if (dev.base == baseaddr)
			return dev.device;
	}
	return nullptr;
}

void Bus::ensureSorted() const {
	if (!devices_sorted) {
		std::sort(const_cast<std::vector<DeviceEntry>&>(myDevices).begin(), 
		          const_cast<std::vector<DeviceEntry>&>(myDevices).end());
		const_cast<bool&>(devices_sorted) = true;
	}
}

uint64_t Bus::load(uint64_t addr, uint8_t size) const
{
	ensureSorted();
	
	// Use binary search for faster device lookup
	auto it = std::lower_bound(myDevices.begin(), myDevices.end(), addr, 
		[](const DeviceEntry& entry, uint64_t addr) {
			return entry.end <= addr;
		});
	
	if (it != myDevices.end() && it->base <= addr && addr < it->end) {
		return it->device->load(addr - it->base, size);
	}

	throw(CpuException(Except::LoadAccessFault));
}

void Bus::store(uint64_t addr, uint8_t size, uint64_t value)
{
	ensureSorted();
	
	// Use binary search for faster device lookup
	auto it = std::lower_bound(myDevices.begin(), myDevices.end(), addr, 
		[](const DeviceEntry& entry, uint64_t addr) {
			return entry.end <= addr;
		});
	
	if (it != myDevices.end() && it->base <= addr && addr < it->end) {
		it->device->store(addr - it->base, size, value);
		return;
	}

	throw(CpuException(Except::StoreAMOAccessFault));
}
