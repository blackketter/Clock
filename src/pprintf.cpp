#include "pprintf.h"
#include "Print.h"
#include <stdio.h>
#include <stdarg.h>

#if 0
// ESP32/8266 have broken vdprintfs
int pprintf(Print* p, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = vdprintf((int)p, format, ap);
	va_end(ap);
	return result;
}
#endif

int pprintf(Print* p, const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    int len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > (int)sizeof(temp) - 1) {
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

