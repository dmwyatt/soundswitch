#pragma once

#include "Device.h"
#include <stdexcept>
#include <string>
#include <vector>

namespace ap
{
	struct Options
	{
		DeviceFilter Filter;
		bool ShowIds = false;
		bool ShowHelp = false;
	};

	// Thrown for a command line that cannot be understood.
	class UsageError : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};

	extern const char* const USAGE;

	// Parses the command-line arguments that follow the program name.
	Options ParseOptions(const std::vector<std::string>& args);
}
