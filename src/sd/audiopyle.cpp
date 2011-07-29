/* 
Audiopyle
Author: Chris Penrose (chris@pleasesendhelp.com)
Date: July 15 2011
Description: Displays a list of audio devices. Written for Thermopyle as
per request: http://forums.somethingawful.com/showthread.php?threadid=2415898&pagenumber=57#post393575494
*/

#include "DevicePrinter.h"

ap::DevicePrinter::Options GetOptions(int argc, char** argv)
{
	ap::DevicePrinter::Options options;

	for(int i = 0; i < argc; i++)
	{
		if(strcmp(argv[i], "-f") == 0)
		{
			options.StateMask = 0;
			options.DataFlow = 0;
			for(int j = i+1; j < argc; j++)
			{
				if(argv[j][0] == '-') { i = j-1; break; }
				if(strcmp(argv[j], "active") == 0) options.StateMask |= DEVICE_STATE_ACTIVE;
				else if(strcmp(argv[j], "disabled") == 0) options.StateMask |= DEVICE_STATE_DISABLED;
				else if(strcmp(argv[j], "notpresent") == 0) options.StateMask |= DEVICE_STATE_NOTPRESENT;
				else if(strcmp(argv[j], "unplugged") == 0) options.StateMask |= DEVICE_STATE_UNPLUGGED;
				else if(strcmp(argv[j], "capture") == 0) options.DataFlow |= ap::Device::CAPTURE;
				else if(strcmp(argv[j], "render") == 0) options.DataFlow |= ap::Device::RENDER;
			}
			if(options.StateMask == 0) options.StateMask = DEVICE_STATEMASK_ALL;
			if(options.DataFlow == 0) options.DataFlow = ap::Device::CAPTURE | ap::Device::RENDER;
		}
		else if(strcmp(argv[i], "-id") == 0) options.ShowIds = true;
	}

	return options;
}

int main(int argc, char** argv)
{
	ap::DevicePrinter printer(GetOptions(argc, argv));
	printer.Print();

	fflush(stdout);
	return 0;
}