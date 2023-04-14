
#include <os/list.h>
#include <os/assert.h>

ListNode::ListNode() { }

ListNode::ListNode(ListNode *prev, ListNode *next)
    : prev(prev), next(next) { }

ListNode::~ListNode() { }

List::List()
    : head(nullptr, &this->tail), tail(&this->head, nullptr) { }

List::~List() { }

/* 在 anchor 节点前插入 node 节点 */
void List::insert_before(ListNode *anchor, ListNode *node) {
    anchor->prev->next = node;
    node->prev = anchor->prev;
    anchor->prev = node;
    node->next = anchor;
}

/* 在 anchor 节点后插入 node 节点 */
void List::insert_after(ListNode *anchor, ListNode *node) {
    anchor->next->prev = node;
    node->next = anchor->next;
    anchor->next = node;
    node->prev = anchor;
}

/* 在头节点后插入 node 节点 */
void List::push(ListNode *node) {
    head.next->prev = node;
    node->next = head.next;
    head.next = node;
    node->prev = &head;
}

/* 删除头节点后的节点 */
ListNode *List::pop() {
    assert(!empty());

    ListNode *removed = head.next;
    remove(head.next);
    return removed;
}

/* 在尾节点前插入 node 节点 */
void List::push_back(ListNode *node) {
    node->prev = tail.prev;
    tail.prev->next = node;
    node->next = &tail;
    tail.prev = node;
}

/* 删除尾节点前的节点 */
ListNode *List::pop_back() {
    assert(!empty());

    ListNode *removed = tail.prev;
    remove(tail.prev);
    return removed;
}

/* 在链表中查询 node 节点是否存在 */
bool List::search(ListNode *node) {
    ListNode *next = head.next;
    while (next != &tail) {
        if (next == node) return true;
        next = next->next;
    }
    return false;
}

/* 移除链表中的 node 节点 */
void List::remove(ListNode *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = nullptr;
    node->next = nullptr;
}

/* 链表是否为空 */
bool List::empty() {
    return head.next == &tail;
}

/* 获取链表的元素个数 */
u32 List::length() {
    int count = 0;
    ListNode *node = head.next;
    while (node != &tail) {
        node = node->next;
        count++;
    }
    return count;
}

/* 获取链所占用的内存空间大小 */
u32 List::size() {
    return 0;
}
