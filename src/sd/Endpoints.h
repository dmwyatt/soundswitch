#pragma once

#include "Device.h"
#include <vector>

struct IMMDeviceEnumerator;

namespace ap
{
	// Reads the devices matching the filter from Windows.
	// Throws std::system_error when Windows reports a failure.
	std::vector<Device> ListDevices(const DeviceFilter& filter);

	// Reads the devices matching the filter from the given enumerator.
	// Throws std::system_error when the enumerator or one of its devices reports a failure.
	std::vector<Device> ReadDevices(IMMDeviceEnumerator& enumerator, const DeviceFilter& filter);
}
