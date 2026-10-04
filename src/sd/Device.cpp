#include "Device.h"

#include <stdexcept>
#include <tuple>
#include <vector>

namespace ap
{
	namespace
	{
		const std::wstring UNKNOWN = L"[unknown]";

		std::wstring Join(const std::vector<std::wstring>& parts, const wchar_t separator)
		{
			std::wstring joined;
			for(size_t i = 0; i < parts.size(); i++)
			{
				if(i > 0) joined += separator;
				joined += parts[i];
			}
			return joined;
		}

		std::wstring FlowName(const DataFlow flow)
		{
			switch(flow)
			{
			case DataFlow::Render: return L"Render";
			case DataFlow::Capture: return L"Capture";
			}
			throw std::logic_error("Unhandled data flow");
		}

		std::wstring StateName(const std::optional<DeviceState> state)
		{
			if(!state) return UNKNOWN;

			switch(*state)
			{
			case DeviceState::Active: return L"Active";
			case DeviceState::Disabled: return L"Disabled";
			case DeviceState::NotPresent: return L"Not present";
			case DeviceState::Unplugged: return L"Unplugged";
			}
			throw std::logic_error("Unhandled device state");
		}

		std::wstring RoleNames(const DeviceRoles& roles)
		{
			std::vector<std::wstring> names;
			if(roles.Console) names.push_back(L"Console");
			if(roles.Multimedia) names.push_back(L"Multimedia");
			if(roles.Communications) names.push_back(L"Communications");
			return Join(names, L'|');
		}
	}

	DeviceRoles RolesFor(const std::wstring& id, const DefaultDevices& defaults)
	{
		return {
			.Console = defaults.Console == id,
			.Multimedia = defaults.Multimedia == id,
			.Communications = defaults.Communications == id };
	}

	bool ListedBefore(const Device& lhs, const Device& rhs)
	{
		return std::tie(lhs.Flow, lhs.Id) < std::tie(rhs.Flow, rhs.Id);
	}

	std::wstring FormatDevice(const Device& device, const bool showId)
	{
		std::vector<std::wstring> fields;
		if(showId) fields.push_back(device.Id);
		fields.push_back(device.Name.value_or(UNKNOWN));
		fields.push_back(device.Description.value_or(UNKNOWN));
		fields.push_back(FlowName(device.Flow));
		fields.push_back(StateName(device.State));

		const std::wstring roles = RoleNames(device.Roles);
		if(!roles.empty()) fields.push_back(roles);

		return Join(fields, L',');
	}
}
