#ifndef _AP_DEVICEPRINTER_H
#define _AP_DEVICEPRINTER_H

#include <windows.h>
#include "Device.h"
#include "DefaultDevices.h"
#include <set>


namespace ap
{
	class DevicePrinter
	{
	public:
		struct Options
		{
			DWORD StateMask;
			DWORD DataFlow;
			bool ShowIds;
			Options() : StateMask(DEVICE_STATEMASK_ALL), DataFlow(Device::CAPTURE | Device::RENDER), ShowIds(false) {}
		};
		DevicePrinter(const Options& options);
		bool Print();
	private:

		std::set<Device> mDevices;
		DefaultDevices mDefaultRenderDevices;
		DefaultDevices mDefaultCaptureDevices;
		const Options mOptions;

		void GetDefaultDevice(IMMDeviceEnumerator* const pEnumerator, const EDataFlow dataFlow, const ERole role, wchar_t** id) const;
		void GetDefaultDevices(IMMDeviceEnumerator* const pEnumerator);
		bool GetDevices(IMMDeviceEnumerator* const pEnumerator, const EDataFlow dataFlow);
	};
}

#endif