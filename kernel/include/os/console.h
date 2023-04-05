#ifndef _H_CONSOLE
#define _H_CONSOLE

#include <os/types.h>

class Console {
    public:
        static u32 screen; // 当前显示器开始的内存位置
        static u32 pos;    // 当前光标的内存位置
        static u16 x, y;   // 当前光标的坐标位置
        u8 attr;           // 字符样式
        u16 erase;         // 空格

    public:
        Console();
        ~Console();

        void write(char* buf, u32 count);
        void clear();

    protected:
        void get_screen();
        void set_screen();
        void get_cursor();
        void set_cursor();
        void scroll_up();
        void command_lf();
        void command_cr();
        void command_bs();
        void command_del();

};

#endif