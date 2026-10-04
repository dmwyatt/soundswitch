#include <doctest/doctest.h>
#include "Output.h"

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <string>

using namespace ap;

namespace
{
	// Everything WriteLines put into a fresh temporary file, byte for byte.
	std::string BytesWritten(const std::vector<std::wstring>& lines)
	{
		FILE* file = nullptr;
		REQUIRE(tmpfile_s(&file) == 0);
		// The mode stdout starts in.
		REQUIRE(_setmode(_fileno(file), _O_TEXT) != -1);
		WriteLines(file, lines);

		// Read through Windows rather than the C runtime, which would translate the bytes back.
		const HANDLE handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
		REQUIRE(SetFilePointer(handle, 0, nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER);
		char buffer[256];
		DWORD size = 0;
		REQUIRE(ReadFile(handle, buffer, sizeof(buffer), &size, nullptr));
		fclose(file);
		return std::string(buffer, size);
	}
}

TEST_CASE("WriteLines ends each line with a Windows line break")
{
	CHECK(BytesWritten({ L"first", L"second" }) == "first\r\nsecond\r\n");
}

TEST_CASE("WriteLines writes text from any script as UTF-8")
{
	// "café" followed by the Japanese and Russian words for "speakers".
	const std::wstring line = L"café スピーカー Динамики";

	CHECK(BytesWritten({ line }) ==
		"caf\xC3\xA9 "
		"\xE3\x82\xB9\xE3\x83\x94\xE3\x83\xBC\xE3\x82\xAB\xE3\x83\xBC "
		"\xD0\x94\xD0\xB8\xD0\xBD\xD0\xB0\xD0\xBC\xD0\xB8\xD0\xBA\xD0\xB8\r\n");
}

TEST_CASE("WriteLines writes nothing for no lines")
{
	CHECK(BytesWritten({}).empty());
}
