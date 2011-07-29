#include "DevicePrinter.h"
#include "Utils.h"

namespace ap
{
	DevicePrinter::DevicePrinter(const Options& options)
		: mOptions(options)
	{}

	bool DevicePrinter::GetDevices(IMMDeviceEnumerator* const pEnumerator, const EDataFlow dataFlow)
	{
		// Enumerate audio endpoints
		IMMDeviceCollection* pDevices = 0;
		if(FAILED(pEnumerator->EnumAudioEndpoints(dataFlow, mOptions.StateMask, &pDevices)))
		{
			Utils::Error("Failed to enumerate audio endpoints\n");
			return false;
		}

		// Get device count
		unsigned int count = 0;
		if(FAILED(pDevices->GetCount(&count)))
		{
			Utils::Error("Failed to get device count\n");
			pDevices->Release();
			return false;
		}

		if(count == 0)
		{
			pDevices->Release();
			return false;
		}

		// For each device ...
		for(unsigned int i = 0; i < count; i++)
		{
			// Get it
			IMMDevice* pDevice = 0;
			if(FAILED(pDevices->Item(i, &pDevice)))
				Utils::Error("Failed to get device %u of %u\n", i, count);
			else
			{
				Device d(pDevice, dataFlow, (dataFlow == eCapture) ? mDefaultCaptureDevices : mDefaultRenderDevices);
				// Add it to the set
				mDevices.insert(d);
			}
		}

		// Clean up
		pDevices->Release();
		return true;
	}

	bool DevicePrinter::Print()
	{
		// Initialize COM library
		if(FAILED(CoInitialize(0)))
		{
			Utils::FatalError("Failed to initialize COM library\n");
			return false;
		}
		
		// Create device enumerator
		IMMDeviceEnumerator* pEnumerator = 0;
		if(FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator)))
		{
			Utils::FatalError("Failed to get device enumerator\n");
			return false;
		}

		// Get default devices
		GetDefaultDevices(pEnumerator);

		// Get devices
		if((mOptions.DataFlow & Device::CAPTURE) == Device::CAPTURE) GetDevices(pEnumerator, eCapture);
		if((mOptions.DataFlow & Device::RENDER) == Device::RENDER) GetDevices(pEnumerator, eRender);
		
		
		// Print devices
		for(std::set<Device>::iterator device = mDevices.begin();
			device != mDevices.end();
			device++)
		{
			wprintf(L"%s\n", device->ToString(mOptions.ShowIds).c_str());
		}

		// Clean up
		pEnumerator->Release();
		CoUninitialize();

		return true;
	}

	void DevicePrinter::GetDefaultDevice(IMMDeviceEnumerator* const pEnumerator, const EDataFlow dataFlow, const ERole role, wchar_t** id) const
	{
		IMMDevice* device;
		if(SUCCEEDED(pEnumerator->GetDefaultAudioEndpoint(dataFlow, role, &device))) device->GetId(id);
	}
	void DevicePrinter::GetDefaultDevices(IMMDeviceEnumerator* const pEnumerator)
	{
		GetDefaultDevice(pEnumerator, eRender, eConsole, &mDefaultRenderDevices.Console);
		GetDefaultDevice(pEnumerator, eCapture, eConsole, &mDefaultCaptureDevices.Console);
		GetDefaultDevice(pEnumerator, eRender, eMultimedia, &mDefaultRenderDevices.Multimedia);
		GetDefaultDevice(pEnumerator, eCapture, eMultimedia, &mDefaultCaptureDevices.Multimedia);
		GetDefaultDevice(pEnumerator, eRender, eCommunications, &mDefaultRenderDevices.Communications);
		GetDefaultDevice(pEnumerator, eCapture, eCommunications, &mDefaultCaptureDevices.Communications);
	}
}