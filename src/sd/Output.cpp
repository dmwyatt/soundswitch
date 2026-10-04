#include "Output.h"

#include <cerrno>
#include <fcntl.h>
#include <io.h>
#include <system_error>

namespace ap
{
	namespace
	{
		void ThrowIf(const bool failed, const char* const action)
		{
			if(failed) throw std::system_error(errno, std::generic_category(), action);
		}
	}

	void WriteLines(FILE* const stream, const std::vector<std::wstring>& lines)
	{
		// In its default mode the C runtime drops text that the system code page cannot hold.
		ThrowIf(_setmode(_fileno(stream), _O_U8TEXT) == -1, "Failed to switch the output to UTF-8");

		for(const std::wstring& line : lines)
		{
			ThrowIf(fputws((line + L'\n').c_str(), stream) < 0, "Failed to write output");
		}
		ThrowIf(fflush(stream) != 0, "Failed to write output");
	}
}
