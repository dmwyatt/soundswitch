#ifndef _AP_UTILS_H
#define _AP_UTILS_H

#include <stdio.h>

namespace ap
{
	namespace Utils
	{
		void FatalError(const char* const err, ...);
		void Error(const char* const err, ...);
	}
}

#endif