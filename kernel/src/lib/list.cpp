
#include <os/list.h>
#include <os/assert.h>

// 初始化链表
void list_init(list_t *list)
{
    list->head.prev = nullptr;
    list->tail.next = nullptr;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
}

// 在 anchor 结点前插入结点 node
void list_insert_before(list_node_t *anchor, list_node_t *node)
{
    node->prev = anchor->prev;
    node->next = anchor;

    anchor->prev->next = node;
    anchor->prev = node;
}

// 在 anchor 结点后插入结点 node
void list_insert_after(list_node_t *anchor, list_node_t *node)
{
    node->prev = anchor;
    node->next = anchor->next;

    anchor->next->prev = node;
    anchor->next = node;
}

// 插入到头结点后
void list_push(list_t *list, list_node_t *node)
{
    assert(!list_search(list, node));
    list_insert_after(&list->head, node);
}

// 移除头结点后的结点
list_node_t *list_pop(list_t *list)
{
    assert(!list_empty(list));

    list_node_t *node = list->head.next;
    list_remove(node);

    return node;
}

// 插入到尾结点前
void list_pushback(list_t *list, list_node_t *node)
{
    assert(!list_search(list, node));
    list_insert_before(&list->tail, node);
}

// 移除尾结点前的结点
list_node_t *list_popback(list_t *list)
{
    assert(!list_empty(list));

    list_node_t *node = list->tail.prev;
    list_remove(node);

    return node;
}

// 查找链表中结点是否存在
bool list_search(list_t *list, list_node_t *node)
{
    list_node_t *next = list->head.next;
    while (next != &list->tail)
    {
        if (next == node)
            return true;
        next = next->next;
    }
    return false;
}

// 从链表中删除结点
void list_remove(list_node_t *node)
{
    assert(node->prev != nullptr);
    assert(node->next != nullptr);

    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = nullptr;
    node->prev = nullptr;
}

// 判断链表是否为空
bool list_empty(list_t *list)
{
    return (list->head.next == &list->tail);
}

// 获得链表长度
u32 list_size(list_t *list)
{
    list_node_t *next = list->head.next;
    u32 size = 0;
    while (next != &list->tail)
    {
        size++;
        next = next->next;
    }
    return size;
}

#include <os/memory.h>
#include <os/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

extern "C" void list_test(KernelVirtualMemory kvm)
{
    u32 count = 3;
    List holder = List();
    ListNode *node;

    while (count--)
    {
        node = (ListNode *)kvm.alloc_kpage(1);
        holder.push(node);
    }

    while (!holder.empty())
    {
        node = holder.pop();
        kvm.free_kpage((u32)node, 1);
    }

    count = 3;
    while (count--)
    {
        node = (ListNode *)kvm.alloc_kpage(1);
        holder.push_back(node);
    }

    LOGK("list size %d\n", holder.length());

    while (!holder.empty())
    {
        node = holder.pop_back();
        kvm.free_kpage((u32)node, 1);
    }

    node = (ListNode *)kvm.alloc_kpage(1);
    holder.push_back(node);

    LOGK("search node 0x%p --> %d\n", node, holder.search(node));
    LOGK("search node 0x%p --> %d\n", 0, holder.search(0));

    holder.remove(node);
    kvm.free_kpage((u32)node, 1);
}

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
