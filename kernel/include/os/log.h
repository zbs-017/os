#ifndef _H_LOG
#define _H_LOG

#include <os/stdarg.h>
#include <os/console.h>

class Log {
    public:
        Log();
        ~Log();

        int printk(const char *fmt, ...);

    protected:
        int vsprintf(char *buf, const char *fmt, va_list args);
        int sprintf(char *buf, const char *fmt, ...);

    private:
        int skip_atoi(const char **s);
        char *number(char *str, unsigned long num, int base, int size, int precision, int flags);

    private:
        Console console;
        char buf[1024];
};

#endif