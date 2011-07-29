#ifndef _AP_DEFAULTDEVICES_H
#define _AP_DEFAULTDEVICES_H

namespace ap
{
	// Struct of device IDs for the default devices
	struct DefaultDevices
	{
		wchar_t* Console;
		wchar_t* Multimedia;
		wchar_t* Communications;
		DefaultDevices() :
			Console(0),
			Multimedia(0),
			Communications(0) {}
	};
}

#endif