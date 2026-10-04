#pragma once

#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <wrl/implements.h>
#include <cstring>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// In-memory stand-ins for the Windows audio endpoint objects, so the code that
// reads devices can be tested against situations a real machine rarely offers.
namespace ap::fakes
{
	using Microsoft::WRL::ClassicCom;
	using Microsoft::WRL::ComPtr;
	using Microsoft::WRL::Make;
	using Microsoft::WRL::RuntimeClass;
	using Microsoft::WRL::RuntimeClassFlags;

	inline wchar_t* CoTaskMemCopy(const std::wstring& text)
	{
		const size_t bytes = (text.size() + 1) * sizeof(wchar_t);
		wchar_t* const copy = static_cast<wchar_t*>(CoTaskMemAlloc(bytes));
		memcpy(copy, text.c_str(), bytes);
		return copy;
	}

	struct EndpointSpec
	{
		std::wstring Id;
		EDataFlow Flow = eRender;
		DWORD State = DEVICE_STATE_ACTIVE;
		std::optional<std::wstring> Name = L"Adapter";
		std::optional<std::wstring> Description = L"Endpoint";
	};

	class FakePropertyStore : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IPropertyStore>
	{
	public:
		explicit FakePropertyStore(EndpointSpec spec) : mSpec(std::move(spec)) {}

		IFACEMETHODIMP GetCount(DWORD*) override { return E_NOTIMPL; }
		IFACEMETHODIMP GetAt(DWORD, PROPERTYKEY*) override { return E_NOTIMPL; }
		IFACEMETHODIMP GetValue(REFPROPERTYKEY key, PROPVARIANT* result) override
		{
			PropVariantInit(result);
			const std::optional<std::wstring>& text = IsEqualPropertyKey(key, PKEY_DeviceInterface_FriendlyName) ? mSpec.Name : mSpec.Description;
			if(text)
			{
				result->vt = VT_LPWSTR;
				result->pwszVal = CoTaskMemCopy(*text);
			}
			return S_OK;
		}
		IFACEMETHODIMP SetValue(REFPROPERTYKEY, REFPROPVARIANT) override { return E_NOTIMPL; }
		IFACEMETHODIMP Commit() override { return E_NOTIMPL; }

	private:
		const EndpointSpec mSpec;
	};

	class FakeEndpoint : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMMDevice>
	{
	public:
		explicit FakeEndpoint(EndpointSpec spec) : Spec(std::move(spec)) {}

		const EndpointSpec Spec;

		IFACEMETHODIMP Activate(REFIID, DWORD, PROPVARIANT*, void**) override { return E_NOTIMPL; }
		IFACEMETHODIMP OpenPropertyStore(DWORD, IPropertyStore** store) override
		{
			return Make<FakePropertyStore>(Spec).CopyTo(store);
		}
		IFACEMETHODIMP GetId(LPWSTR* id) override
		{
			*id = CoTaskMemCopy(Spec.Id);
			return S_OK;
		}
		IFACEMETHODIMP GetState(DWORD* state) override
		{
			*state = Spec.State;
			return S_OK;
		}
	};

	class FakeCollection : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMMDeviceCollection>
	{
	public:
		explicit FakeCollection(std::vector<ComPtr<FakeEndpoint>> endpoints) : mEndpoints(std::move(endpoints)) {}

		IFACEMETHODIMP GetCount(UINT* count) override
		{
			*count = static_cast<UINT>(mEndpoints.size());
			return S_OK;
		}
		IFACEMETHODIMP Item(UINT index, IMMDevice** endpoint) override
		{
			return mEndpoints.at(index).CopyTo(endpoint);
		}

	private:
		const std::vector<ComPtr<FakeEndpoint>> mEndpoints;
	};

	class FakeEnumerator : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMMDeviceEnumerator>
	{
	public:
		std::vector<ComPtr<FakeEndpoint>> Endpoints;
		// The default device for a flow and role. A missing entry means Windows has none.
		std::map<std::pair<EDataFlow, ERole>, ComPtr<FakeEndpoint>> Defaults;
		HRESULT EnumerateResult = S_OK;

		ComPtr<FakeEndpoint> Add(EndpointSpec spec)
		{
			Endpoints.push_back(Make<FakeEndpoint>(std::move(spec)));
			return Endpoints.back();
		}

		IFACEMETHODIMP EnumAudioEndpoints(EDataFlow flow, DWORD stateMask, IMMDeviceCollection** collection) override
		{
			if(FAILED(EnumerateResult)) return EnumerateResult;

			std::vector<ComPtr<FakeEndpoint>> matching;
			for(const ComPtr<FakeEndpoint>& endpoint : Endpoints)
			{
				if(endpoint->Spec.Flow == flow && (endpoint->Spec.State & stateMask) != 0) matching.push_back(endpoint);
			}
			return Make<FakeCollection>(std::move(matching)).CopyTo(collection);
		}
		IFACEMETHODIMP GetDefaultAudioEndpoint(EDataFlow flow, ERole role, IMMDevice** endpoint) override
		{
			*endpoint = nullptr;
			const auto found = Defaults.find({ flow, role });
			if(found == Defaults.end()) return E_NOTFOUND;
			return found->second.CopyTo(endpoint);
		}
		IFACEMETHODIMP GetDevice(LPCWSTR, IMMDevice**) override { return E_NOTIMPL; }
		IFACEMETHODIMP RegisterEndpointNotificationCallback(IMMNotificationClient*) override { return E_NOTIMPL; }
		IFACEMETHODIMP UnregisterEndpointNotificationCallback(IMMNotificationClient*) override { return E_NOTIMPL; }
	};
}
