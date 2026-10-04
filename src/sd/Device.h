#pragma once

#include <optional>
#include <set>
#include <string>

namespace ap
{
	// Declaration order is listing order: render devices are listed before capture devices.
	enum class DataFlow
	{
		Render,
		Capture
	};

	enum class DeviceState
	{
		Active,
		Disabled,
		NotPresent,
		Unplugged
	};

	// A device is listed when its state and its data flow are both in these sets.
	struct DeviceFilter
	{
		std::set<DeviceState> States = { DeviceState::Active, DeviceState::Disabled, DeviceState::NotPresent, DeviceState::Unplugged };
		std::set<DataFlow> Flows = { DataFlow::Render, DataFlow::Capture };

		bool operator==(const DeviceFilter&) const = default;
	};

	// The ID of the default device for each role within one data flow.
	// A role is empty when Windows has no default device for it.
	struct DefaultDevices
	{
		std::optional<std::wstring> Console;
		std::optional<std::wstring> Multimedia;
		std::optional<std::wstring> Communications;
	};

	// The roles a device is the default device for.
	struct DeviceRoles
	{
		bool Console = false;
		bool Multimedia = false;
		bool Communications = false;
	};

	struct Device
	{
		std::wstring Id;
		// Empty when Windows has no such property for the device.
		std::optional<std::wstring> Name;
		std::optional<std::wstring> Description;
		DataFlow Flow = DataFlow::Render;
		// Empty when Windows reports a state this program does not know.
		std::optional<DeviceState> State;
		DeviceRoles Roles;
	};

	DeviceRoles RolesFor(const std::wstring& id, const DefaultDevices& defaults);

	// Orders devices by data flow, then by ID.
	bool ListedBefore(const Device& lhs, const Device& rhs);

	// One line of output: [ID,]NAME,DESCRIPTION,FLOW,STATE[,ROLE|ROLE...]
	// Control and text-direction characters in a field become spaces, so a device is always exactly one line.
	// A field holding a comma or quote is quoted as in CSV.
	std::wstring FormatDevice(const Device& device, bool showId);
}
