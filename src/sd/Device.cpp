#include "Device.h"
#include <sstream>

namespace ap
{
	const wchar_t* const Device::UNKNOWN_PROPERTY = L"[unknown]";

	Device::Device(IMMDevice* const pDevice, const EDataFlow dataFlow, const DefaultDevices& defaultDevices)
		: mDataFlow(dataFlow), mRoles(0)
	{
		GetState(pDevice);
		GetId(pDevice);
		GetProperties(pDevice);
		GetRoles(defaultDevices);
	}

	void Device::GetPropertyString(IPropertyStore* const pProps, REFPROPERTYKEY key, wchar_t** string) const
	{
		PROPVARIANT prop;
		if(FAILED(pProps->GetValue(key, &prop))) *string = L"[unknown]";
		else *string = prop.pwszVal;
	}

	void Device::GetRoles(const DefaultDevices& defaultDevices)
	{
		if(mId.compare(defaultDevices.Console) == 0) mRoles |= CONSOLE;
		if(mId.compare(defaultDevices.Multimedia) == 0) mRoles |= MULTIMEDIA;
		if(mId.compare(defaultDevices.Communications) == 0) mRoles |= COMMUNICATIONS;
	}

	std::wstring Device::RoleString() const
	{
		bool sep = false;
		std::wstring out;
		if((mRoles & CONSOLE) == CONSOLE) { out.append(L",Console"); sep = true; }
		if((mRoles & MULTIMEDIA) == MULTIMEDIA) { sep ? out.push_back('|') : out.push_back(','); out.append(L"Multimedia"); sep = true; }	
		if((mRoles & COMMUNICATIONS) == COMMUNICATIONS) { sep ? out.push_back('|') : out.push_back(','); out.append(L"Communications"); sep = true; }
		return out;
	}
	
	void Device::GetProperties(IMMDevice* const pDevice)
	{
		// Open property store
		IPropertyStore* props = 0;
		if(FAILED(pDevice->OpenPropertyStore(STGM_READ, &props)))
		{
			mName = UNKNOWN_PROPERTY;
			mDesc = UNKNOWN_PROPERTY;
			return;
		}

		// Get friendly name
		wchar_t* sName = 0;
		GetPropertyString(props, PKEY_DeviceInterface_FriendlyName, &sName);
		mName = sName;

		// Get description
		wchar_t* sDesc = 0;
		GetPropertyString(props, PKEY_Device_DeviceDesc, &sDesc);
		mDesc = sDesc;
	}

	void Device::GetState(IMMDevice* const pDevice)
	{
		DWORD dwState;
		if(FAILED(pDevice->GetState(&dwState))) { mState = UNKNOWN; return; }

		switch(dwState)
		{
		case DEVICE_STATE_ACTIVE: mState = ACTIVE; return;
		case DEVICE_STATE_DISABLED: mState = DISABLED; return;
		case DEVICE_STATE_NOTPRESENT: mState = NOTPRESENT; return;
		case DEVICE_STATE_UNPLUGGED: mState = UNPLUGGED; return;
		default: mState = UNKNOWN; return;
		}
	}
	void Device::GetId(IMMDevice* const pDevice)
	{
		wchar_t* sId = 0;
		if(FAILED(pDevice->GetId(&sId))) { mId = UNKNOWN_PROPERTY; return; }
		mId = sId;
	}
	std::wstring Device::StateString() const
	{
		switch(mState)
		{
		case ACTIVE: return L"Active";
		case DISABLED: return L"Disabled";
		case NOTPRESENT: return L"Not present";
		case UNPLUGGED: return L"Unplugged";
		default: return UNKNOWN_PROPERTY;
		}
	}
	std::wstring Device::DataFlowString() const
	{
		switch(mDataFlow)
		{
		case eCapture: return L"Capture";
		case eRender: return L"Render";
		default: return UNKNOWN_PROPERTY;
		}
	}
	std::wstring Device::ToString(const bool showId) const
	{
		std::wstringstream ss;
		if(showId) ss << mId << ',';
		ss << mName << ',' << mDesc << ',' << DataFlowString() << ',' << StateString() << RoleString();
		return ss.str();
	}
	Device::Device(const Device& rhs)
		: mId(rhs.mId), mName(rhs.mName), mDesc(rhs.mDesc), mDataFlow(rhs.mDataFlow), mRoles(rhs.mRoles), mState(rhs.mState)
	{}
	bool Device::operator<(const Device& rhs) const
	{
		if(mDataFlow < rhs.mDataFlow) return true;
		if(mId.compare(rhs.mId) < 0) return true;
		return false;
	}
	
}