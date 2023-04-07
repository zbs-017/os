#ifndef _OS_TYPES_H
#define _OS_TYPES_H

#include <os/os.h>

#define EOF -1 // END OF FILE
#define EOS '\0' // 字符串结尾

// 用于定义特殊的结构体
#define _packed __attribute__((packed))

// 用于省略函数的栈帧
#define _ofp __attribute__((optimize("omit-frame-pointer")))

typedef unsigned int size_t;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef u32 time_t;
typedef u32 idx_t;

#endif