/*
Audiopyle
Author: Chris Penrose (chris@pleasesendhelp.com)
Date: July 15 2011
Description: Displays a list of audio devices. Written for Thermopyle as
per request: http://forums.somethingawful.com/showthread.php?threadid=2415898&pagenumber=57#post393575494
*/

#include "Endpoints.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <exception>

namespace
{
	struct Options
	{
		ap::DeviceFilter Filter;
		bool ShowIds = false;
	};

	Options GetOptions(int argc, char** argv)
	{
		Options options;

		for(int i = 0; i < argc; i++)
		{
			if(strcmp(argv[i], "-f") == 0)
			{
				ap::DeviceFilter filter = { .States = {}, .Flows = {} };
				for(int j = i+1; j < argc; j++)
				{
					if(argv[j][0] == '-') { i = j-1; break; }
					if(strcmp(argv[j], "active") == 0) filter.States.insert(ap::DeviceState::Active);
					else if(strcmp(argv[j], "disabled") == 0) filter.States.insert(ap::DeviceState::Disabled);
					else if(strcmp(argv[j], "notpresent") == 0) filter.States.insert(ap::DeviceState::NotPresent);
					else if(strcmp(argv[j], "unplugged") == 0) filter.States.insert(ap::DeviceState::Unplugged);
					else if(strcmp(argv[j], "capture") == 0) filter.Flows.insert(ap::DataFlow::Capture);
					else if(strcmp(argv[j], "render") == 0) filter.Flows.insert(ap::DataFlow::Render);
				}
				if(filter.States.empty()) filter.States = ap::DeviceFilter().States;
				if(filter.Flows.empty()) filter.Flows = ap::DeviceFilter().Flows;
				options.Filter = filter;
			}
			else if(strcmp(argv[i], "-id") == 0) options.ShowIds = true;
		}

		return options;
	}
}

int main(int argc, char** argv)
{
	try
	{
		const Options options = GetOptions(argc, argv);

		std::vector<ap::Device> devices = ap::ListDevices(options.Filter);
		std::sort(devices.begin(), devices.end(), ap::ListedBefore);
		for(const ap::Device& device : devices)
		{
			wprintf(L"%ls\n", ap::FormatDevice(device, options.ShowIds).c_str());
		}
		return 0;
	}
	catch(const std::exception& error)
	{
		fprintf(stderr, "sd: %s\n", error.what());
		return 1;
	}
}
