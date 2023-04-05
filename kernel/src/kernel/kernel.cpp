#include <os/os.h>
#include <os/string.h>

#define CRT_ADDR 0x3d4
#define CRT_DATA 0x3d5
#define CRT_CURS_H 0xe
#define CRT_CURS_L 0xf

extern "C" void kernel_init() {
    /* 测试 字符串函数 */
    char message[] = "hello onix!!!";
    char buf[1024];
    int res;

    res = String::strcmp(buf, message);
    String::strcpy(buf, message);
    res = String::strcmp(buf, message);
    String::strcat(buf, message);
    res = String::strcmp(buf, message);
    res = String::strlen(message);
    res = sizeof(message);

    char* ptr = String::strchr(message, '!');
    ptr = String::strrchr(message, '!');

    String::memset(buf, 0, sizeof(buf));
    res = String::memcmp(buf, message, sizeof(message));
    String::memcpy(buf, message, sizeof(message));
    res = String::memcmp(buf, message, sizeof(message));
    ptr = (char*) String::memchr(buf, '!', sizeof(message));
}