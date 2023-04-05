#include <os/debug.h>
#include <os/log.h>

static char buf[1024];

void debugk(char *file, int line, const char *fmt, ...)
{
    Log log = Log();
    va_list args;
    va_start(args, fmt);
    log.vsprintf(buf, fmt, args);
    va_end(args);

    log.printk("[%s] [%d] %s", file, line, buf);
}