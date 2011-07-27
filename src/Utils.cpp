#include "Utils.h"
#include <stdarg.h>

namespace ap
{
	namespace Utils
	{
		void FatalError(const char* const err, ...)
		{
			va_list args;
			va_start(args, err);
			printf("** Fatal Error **\n");
			vprintf(err, args);
			va_end (args);
		}
		void Error(const char* const err, ...)
		{
			va_list args;
			va_start(args, err);
			printf("Error: ");
			vprintf(err, args);
			va_end (args);
		}
	}
}
