#ifndef _H_DEBUG
#define _H_DEBUG

void debugk(char *file, int line, const char *fmt, ...);

#define BMB asm volatile("xchgw %bx, %bx") // bochs magic breakpoint
#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)

#endif