#include "Endpoints.h"

#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <format>
#include <memory>
#include <system_error>
#include <utility>

namespace ap
{
	namespace
	{
		using Microsoft::WRL::ComPtr;

		const std::pair<DeviceState, DWORD> ENDPOINT_STATES[] = {
			{ DeviceState::Active, DEVICE_STATE_ACTIVE },
			{ DeviceState::Disabled, DEVICE_STATE_DISABLED },
			{ DeviceState::NotPresent, DEVICE_STATE_NOTPRESENT },
			{ DeviceState::Unplugged, DEVICE_STATE_UNPLUGGED } };

		void ThrowIfFailed(const HRESULT result, const char* const action)
		{
			if(FAILED(result))
			{
				const std::string what = std::format("{} (0x{:08X})", action, static_cast<unsigned long>(result));
				throw std::system_error(result, std::system_category(), what);
			}
		}

		// Keeps COM initialized on this thread for as long as it lives.
		class ComApartment
		{
		public:
			ComApartment() { ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED), "Failed to initialize COM"); }
			~ComApartment() { CoUninitialize(); }
			ComApartment(const ComApartment&) = delete;
			ComApartment& operator=(const ComApartment&) = delete;
		};

		// Owns a PROPVARIANT and whatever Windows allocates into it.
		class PropVariant
		{
		public:
			PropVariant() { PropVariantInit(&Value); }
			~PropVariant() { PropVariantClear(&Value); }
			PropVariant(const PropVariant&) = delete;
			PropVariant& operator=(const PropVariant&) = delete;

			PROPVARIANT Value;
		};

		struct CoTaskMemFreer
		{
			void operator()(wchar_t* const text) const { CoTaskMemFree(text); }
		};

		EDataFlow EndpointFlow(const DataFlow flow)
		{
			return flow == DataFlow::Render ? eRender : eCapture;
		}

		DWORD StateMask(const std::set<DeviceState>& states)
		{
			DWORD mask = 0;
			for(const auto& [state, endpointState] : ENDPOINT_STATES)
			{
				if(states.contains(state)) mask |= endpointState;
			}
			return mask;
		}

		std::wstring ReadId(IMMDevice& endpoint)
		{
			LPWSTR id = nullptr;
			ThrowIfFailed(endpoint.GetId(&id), "Failed to read a device ID");
			const std::unique_ptr<wchar_t, CoTaskMemFreer> owned(id);
			return owned.get();
		}

		std::optional<DeviceState> ReadState(IMMDevice& endpoint)
		{
			DWORD endpointState = 0;
			ThrowIfFailed(endpoint.GetState(&endpointState), "Failed to read a device state");
			for(const auto& [state, known] : ENDPOINT_STATES)
			{
				if(known == endpointState) return state;
			}
			return std::nullopt;
		}

		std::optional<std::wstring> ReadStringProperty(IPropertyStore& properties, const PROPERTYKEY& key)
		{
			// Windows reports success with an empty value for a property the device does not have.
			PropVariant property;
			ThrowIfFailed(properties.GetValue(key, &property.Value), "Failed to read a device property");
			if(property.Value.vt != VT_LPWSTR) return std::nullopt;
			return property.Value.pwszVal;
		}

		std::optional<std::wstring> ReadDefaultId(IMMDeviceEnumerator& enumerator, const EDataFlow flow, const ERole role)
		{
			ComPtr<IMMDevice> endpoint;
			const HRESULT result = enumerator.GetDefaultAudioEndpoint(flow, role, &endpoint);
			if(result == E_NOTFOUND) return std::nullopt;
			ThrowIfFailed(result, "Failed to find a default device");
			return ReadId(*endpoint.Get());
		}

		DefaultDevices ReadDefaults(IMMDeviceEnumerator& enumerator, const EDataFlow flow)
		{
			return {
				.Console = ReadDefaultId(enumerator, flow, eConsole),
				.Multimedia = ReadDefaultId(enumerator, flow, eMultimedia),
				.Communications = ReadDefaultId(enumerator, flow, eCommunications) };
		}

		Device ReadDevice(IMMDevice& endpoint, const DataFlow flow, const DefaultDevices& defaults)
		{
			ComPtr<IPropertyStore> properties;
			ThrowIfFailed(endpoint.OpenPropertyStore(STGM_READ, &properties), "Failed to open a device's properties");

			const std::wstring id = ReadId(endpoint);
			return {
				.Id = id,
				.Name = ReadStringProperty(*properties.Get(), PKEY_DeviceInterface_FriendlyName),
				.Description = ReadStringProperty(*properties.Get(), PKEY_Device_DeviceDesc),
				.Flow = flow,
				.State = ReadState(endpoint),
				.Roles = RolesFor(id, defaults) };
		}

		std::vector<Device> ReadFlow(IMMDeviceEnumerator& enumerator, const DataFlow flow, const DWORD stateMask)
		{
			const EDataFlow endpointFlow = EndpointFlow(flow);
			ComPtr<IMMDeviceCollection> endpoints;
			ThrowIfFailed(enumerator.EnumAudioEndpoints(endpointFlow, stateMask, &endpoints), "Failed to enumerate audio devices");
			UINT count = 0;
			ThrowIfFailed(endpoints->GetCount(&count), "Failed to count audio devices");

			const DefaultDevices defaults = ReadDefaults(enumerator, endpointFlow);
			std::vector<Device> devices;
			for(UINT i = 0; i < count; i++)
			{
				ComPtr<IMMDevice> endpoint;
				ThrowIfFailed(endpoints->Item(i, &endpoint), "Failed to get an audio device");
				devices.push_back(ReadDevice(*endpoint.Get(), flow, defaults));
			}
			return devices;
		}
	}

	std::vector<Device> ListDevices(const DeviceFilter& filter)
	{
		const ComApartment apartment;
		ComPtr<IMMDeviceEnumerator> enumerator;
		ThrowIfFailed(
			CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&enumerator)),
			"Failed to create the device enumerator");

		return ReadDevices(*enumerator.Get(), filter);
	}

	std::vector<Device> ReadDevices(IMMDeviceEnumerator& enumerator, const DeviceFilter& filter)
	{
		const DWORD stateMask = StateMask(filter.States);
		std::vector<Device> devices;
		for(const DataFlow flow : filter.Flows)
		{
			const std::vector<Device> flowDevices = ReadFlow(enumerator, flow, stateMask);
			devices.insert(devices.end(), flowDevices.begin(), flowDevices.end());
		}
		return devices;
	}
}
