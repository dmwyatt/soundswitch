#include "Device.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <tuple>
#include <utility>
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

		// Inclusive ranges of the characters that act on more than the spot they occupy.
		const std::pair<wchar_t, wchar_t> UNCONFINED[] = {
			{ L'\x0000', L'\x001F' },  // control characters
			{ L'\x007F', L'\x009F' },  // delete and the second block of control characters
			{ L'\x2028', L'\x2029' },  // line and paragraph separators
			{ L'\x202A', L'\x202E' },  // text-direction embeddings and overrides
			{ L'\x2066', L'\x2069' } };  // text-direction isolates

		bool IsUnconfined(const wchar_t character)
		{
			const auto holds = [character](const std::pair<wchar_t, wchar_t>& range)
			{
				return character >= range.first && character <= range.second;
			};
			return std::any_of(std::begin(UNCONFINED), std::end(UNCONFINED), holds);
		}

		// Device text comes from hardware and drivers, so it is not trusted to stay in its field.
		// Characters that could start a new line, drive a terminal or reorder the text around them
		// become spaces, and a field holding a comma or quote is quoted the way CSV quotes it.
		std::wstring Field(std::wstring text)
		{
			std::replace_if(text.begin(), text.end(), IsUnconfined, L' ');
			if(text.find_first_of(L",\"") == std::wstring::npos) return text;

			std::wstring quoted = L"\"";
			for(const wchar_t character : text)
			{
				if(character == L'"') quoted += L'"';
				quoted += character;
			}
			return quoted + L'"';
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
		if(showId) fields.push_back(Field(device.Id));
		fields.push_back(Field(device.Name.value_or(UNKNOWN)));
		fields.push_back(Field(device.Description.value_or(UNKNOWN)));
		fields.push_back(FlowName(device.Flow));
		fields.push_back(StateName(device.State));

		const std::wstring roles = RoleNames(device.Roles);
		if(!roles.empty()) fields.push_back(roles);

		return Join(fields, L',');
	}
}
