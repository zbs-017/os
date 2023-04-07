#include <os/bitmap.h>
#include <os/string.h>
#include <os/assert.h>

Bitmap::Bitmap() { }

Bitmap::~Bitmap() { }

void Bitmap::init(u8* bits, u32 length, u32 offset) {
    this->bits = bits;
    this->length = length;
    this->offset = offset;
    String::memset(this->bits, 0, this->length);
}

bool Bitmap::test(u32 index) {
    assert(index >= this->offset);

    // 得到位图的索引
    idx_t idx = index - this->offset;

    // 位图数组中的字节
    u32 bytes = idx / 8;

    // 该字节中的那一位
    u8 bits = idx % 8;

    assert(bytes < this->length);

    // 返回那一位是否等于 1
    return (bool)(this->bits[bytes] & (1 << bits));
}

void Bitmap::set(u32 index, bool value) {
    // value 必须是二值的
    assert(value == 0 || value == 1);

    assert(index >= this->offset);

    // 得到位图的索引
    idx_t idx = index - this->offset;

    // 位图数组中的字节
    u32 bytes = idx / 8;

    // 该字节中的那一位
    u8 bits = idx % 8;
    if (value)
    {
        // 置为 1
        this->bits[bytes] |= (1 << bits);
    }
    else
    {
        // 置为 0
        this->bits[bytes] &= ~(1 << bits);
    }
}

int Bitmap::scan(u32 count) {
    int start = EOF;                 // 标记目标开始的位置
    u32 bits_left = this->length * 8; // 剩余的位数
    u32 next_bit = 0;                // 下一个位
    u32 counter = 0;                 // 计数器

    // 从头开始找
    while (bits_left-- > 0)
    {
        if (!test(this->offset + next_bit))
        {
            // 如果下一个位没有占用，则计数器加一
            counter++;
        }
        else
        {
            // 否则计数器置为 0，继续寻找
            counter = 0;
        }

        // 下一位，位置加一
        next_bit++;

        // 找到数量一致，则设置开始的位置，结束
        if (counter == count)
        {
            start = next_bit - count;
            break;
        }
    }

    // 如果没找到，则返回 EOF(END OF FILE)
    if (start == EOF)
        return EOF;

    // 否则将找到的位，全部置为 1
    bits_left = count;
    next_bit = start;
    while (bits_left--)
    {
        set(this->offset + next_bit, true);
        next_bit++;
    }

    // 然后返回索引
    return start + this->offset;
}
