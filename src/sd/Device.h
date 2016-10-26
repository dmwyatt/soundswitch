#ifndef _AP_DEVICE_H
#define _AP_DEVICE_H

#include <mmdeviceapi.h>
#include <string>
#include "DefaultDevices.h"
#include <Functiondiscoverykeys_devpkey.h>

namespace ap
{
	class Device
	{
	public:
		Device(
			IMMDevice* const pDevice,
			const EDataFlow dataFlow,
			const DefaultDevices& defaultDevices);
		Device(const Device& rhs);
		std::wstring ToString(const bool showId = false) const;
		enum State
		{
			UNKNOWN = 0,
			ACTIVE,
			DISABLED,
			NOTPRESENT,
			UNPLUGGED
		};
		enum DataFlow
		{
			CAPTURE = 1,
			RENDER = 2
		};
		enum Role
		{
			CONSOLE = 1,
			MULTIMEDIA = 2,
			COMMUNICATIONS = 4
		};
		bool operator<(const Device& rhs) const;
	private:
		static const wchar_t* const UNKNOWN_PROPERTY;

		void GetState(IMMDevice* const pDevice);
		void GetId(IMMDevice* const pDevice);
		void GetProperties(IMMDevice* const pDevice);
		void GetRoles(const DefaultDevices& defaultDevices);
		void GetPropertyString(IPropertyStore* const pProps, REFPROPERTYKEY key, wchar_t** string) const;
		std::wstring StateString() const;
		std::wstring DataFlowString() const;
		std::wstring RoleString() const;
		std::wstring mId;
		std::wstring mName;
		std::wstring mDesc;
		const EDataFlow mDataFlow;
		DWORD mRoles;
		State mState;

	};

}

#endif