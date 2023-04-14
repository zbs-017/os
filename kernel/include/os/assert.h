#ifndef _H_ASSERT
#define _H_ASSERT

#include <os/types.h>
#include <os/log.h>

void assertion_failure(const char *exp, const char *file, const char *base, int line);

#define assert(exp) \
    if (exp)        \
        ;           \
    else            \
        assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

void panic(const char *fmt, ...);

#endif