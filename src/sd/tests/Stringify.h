#pragma once

#include <doctest/doctest.h>
#include <windows.h>
#include <string>

namespace doctest
{
	// Lets a failed check show wide strings instead of "{?}".
	template<> struct StringMaker<std::wstring>
	{
		static String convert(const std::wstring& value)
		{
			const int wideSize = static_cast<int>(value.size());
			const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), wideSize, nullptr, 0, nullptr, nullptr);
			std::string utf8(size, '\0');
			WideCharToMultiByte(CP_UTF8, 0, value.data(), wideSize, utf8.data(), size, nullptr, nullptr);
			return String(utf8.data(), static_cast<String::size_type>(utf8.size()));
		}
	};
}
