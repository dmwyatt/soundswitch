#pragma once

#include <cstdio>
#include <string>
#include <vector>

namespace ap
{
	// Writes each line to the stream, as Unicode to a console and as UTF-8 to a pipe or file.
	// Nothing may have been written to the stream before. Throws std::system_error when writing fails.
	void WriteLines(FILE* stream, const std::vector<std::wstring>& lines);
}
