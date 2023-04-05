#include <os/string.h>

/* 将字符串从 src 拷贝到 dest */
char* String::strcpy(char* dest, const char* src) {
    char* ptr = dest;
    while (true)
    {
        *ptr++ = *src;
        if (*src++ == EOS)
            return dest;
    }
}

/* 将字符串 2 拼接到字符串 1 */
char* String::strcat(char* dest, const char* src) {
    char* ptr = dest;
    while (*ptr != EOS)
    {
        ptr++;
    }
    while (true)
    {
        *ptr++ = *src;
        if (*src++ == EOS)
        {
            return dest;
        }
    }
}

/* 计算字符串的长度 */
size_t String::strlen(const char* str) {
    char* ptr = (char* )str;
    while (*ptr != EOS)
    {
        ptr++;
    }
    return ptr - str;
}

/* 比较字符串，若返回1，左大于右 */
int String::strcmp(const char* lhs, const char* rhs) {
    while (*lhs == *rhs && *lhs != EOS && *rhs != EOS)
    {
        lhs++;
        rhs++;
    }
    return *lhs < *rhs ? -1 : *lhs > *rhs;
}

/* 查找字符在字符串中第一次出现的位置 */
char* String::strchr(const char* str, int ch) {
    char* ptr = (char* )str;
    while (true)
    {
        if (*ptr == ch)
        {
            return ptr;
        }
        if (*ptr++ == EOS)
        {
            return nullptr;
        }
    }
}

/* 查找字符在字符串中最后一次出现的位置 */
char* String::strrchr(const char* str, int ch) {
    char* last = nullptr;
    char* ptr = (char* )str;
    while (true)
    {
        if (*ptr == ch)
        {
            last = ptr;
        }
        if (*ptr++ == EOS)
        {
            return last;
        }
    }
}

/* 比较两块内存中的字符串是否相等，若返回1，左大于右 */
int String::memcmp(const void* lhs, const void* rhs, size_t count) {
    char* lptr = (char* )lhs;
    char* rptr = (char* )rhs;
    while (*lptr == *rptr && count-- > 0)
    {
        lptr++;
        rptr++;
    }
    return *lptr < *rptr ? -1 : *lptr > *rptr;
}

/* 将指定大小的内存设置为给定的值 */
void* String::memset(void* dest, int ch, size_t count) {
    char* ptr = (char*) dest;
    while (count--)
    {
        *ptr++ = ch;
    }
    return dest;
}

/* 将一块内存中的内容复制到另一块内存中 */
void* String::memcpy(void* dest, const void* src, size_t count) {
    char* ptr = (char*) dest;
    char* s = (char*) src;
    while (count--)
    {
        *ptr++ = *((char* )(s++));
    }
    return dest;
}

/* 查找指定内存中首次出现给定值的位置 */
void* String::memchr(const void* str, int ch, size_t count) {
    char* ptr = (char* )str;
    while (count--)
    {
        if (*ptr == ch)
        {
            return (void* )ptr;
        }
        ptr++;
    }
    return nullptr;
}