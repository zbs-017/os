#ifndef _H_LIST
#define _H_LIST

#include <os/types.h>

#define element_offset(type, member) (u32)(&((type *)0)->member)
#define element_entry(type, member, ptr) (type *)((u32)ptr - element_offset(type, member))

class ListNode {
    public:
        ListNode *prev;
        ListNode *next;

    public:
        ListNode();
        ~ListNode();
};

class List {
    public:
        ListNode head;
        ListNode tail;
    
    public:
        List();
        ~List();

        void insert_before(ListNode *anchor, ListNode *node);
        void insert_after(ListNode *anchor, ListNode *node);
        void push(ListNode *node);
        ListNode *pop();
        void push_back(ListNode *node);
        ListNode *pop_back();
        bool search(ListNode *node);
        void remove(ListNode *node);
        bool empty();
        u32 length();
        u32 size();

};

#endif