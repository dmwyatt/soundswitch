/*
Audiopyle
Author: Chris Penrose (chris@pleasesendhelp.com)
Date: July 15 2011
Description: Displays a list of audio devices. Written for Thermopyle as
per request: http://forums.somethingawful.com/showthread.php?threadid=2415898&pagenumber=57#post393575494
*/

#include "Endpoints.h"
#include "Options.h"

#include <algorithm>
#include <cstdio>
#include <exception>

namespace
{
	const int EXIT_FAILED = 1;
	const int EXIT_USAGE = 2;

	void PrintDevices(const ap::Options& options)
	{
		std::vector<ap::Device> devices = ap::ListDevices(options.Filter);
		std::sort(devices.begin(), devices.end(), ap::ListedBefore);
		for(const ap::Device& device : devices)
		{
			wprintf(L"%ls\n", ap::FormatDevice(device, options.ShowIds).c_str());
		}
	}
}

int main(int argc, char** argv)
{
	try
	{
		const std::vector<std::string> args(argv + 1, argv + argc);
		const ap::Options options = ap::ParseOptions(args);

		if(options.ShowHelp) fputs(ap::USAGE, stdout);
		else PrintDevices(options);
		return 0;
	}
	catch(const ap::UsageError& error)
	{
		fprintf(stderr, "sd: %s\n\n%s", error.what(), ap::USAGE);
		return EXIT_USAGE;
	}
	catch(const std::exception& error)
	{
		fprintf(stderr, "sd: %s\n", error.what());
		return EXIT_FAILED;
	}
}
