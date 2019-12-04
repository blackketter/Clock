#include "pprintf.h"
#include "Print.h"
#include <stdio.h>
#include <stdarg.h>

int pprintf(Print* p, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	return vdprintf((int)p, format, ap);
}

#if 0

size_t pprintf(Print* p, const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = new char[len + 1];
        if (!buffer) {
            return 0;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    len = p->write((const uint8_t*) buffer, len);
    if (buffer != temp) {
        delete[] buffer;
    }
    return len;
}

#endif
