#ifndef _H_STDARG
#define _H_STDARG

typedef char *va_list;  // 保存可变参数指针

// 启用可变参数
#define va_start(ap, v) (ap = (va_list)&v + sizeof(char *))
// 获取参数
#define va_arg(ap, t) (*(t *)((ap += sizeof(char *)) - sizeof(char *)))
// 结束可变参数
#define va_end(ap) (ap = (va_list)0)

#endif