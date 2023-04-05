#include <os/assert.h>
#include <os/log.h>

static u8 buf[1024];

// 强制阻塞
static void spin(char *name)
{
    Log log = Log();
    log.printk("spinning in %s ...\n", name);
    while (true)
        ;
}

void assertion_failure(char *exp, char *file, char *base, int line)
{
    Log log = Log();
    log.printk(
        "\n--> assert(%s) failed!!!\n" \
        "--> file: %s \n" \
        "--> base: %s \n" \
        "--> line: %d \n",
        exp, file, base, line);

    spin("assertion_failure()");

    // 不可能走到这里，否则出错；
    asm volatile("ud2");
}

void panic(const char *fmt, ...)
{
    Log log = Log();
    va_list args;
    va_start(args, fmt);
    int i = log.vsprintf((char*)buf, fmt, args);
    va_end(args);

    log.printk("!!! panic !!!\n--> %s \n", buf);
    spin("panic()");

    // 不可能走到这里，否则出错；
    asm volatile("ud2");
}