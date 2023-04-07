SRC = kernel/src
BUILD = build

# 指定入口地址，内核开始的位置
ENTRY = 0x10000

# 使用交叉编译器
CC = /home/zbs/dennix-toolchain/bin/i686-dennix-g++
LD = /home/zbs/dennix-toolchain/bin/i686-dennix-ld
CRTBEGIN_O := $(shell $(CC) $(CXXFLAGS) -print-file-name=crtbegin.o)
CRTEND_O := $(shell $(CC) $(CXXFLAGS) -print-file-name=crtend.o)
START_OBJ := $(BUILD)/boot/crti.o $(CRTBEGIN_O)
END_OBJ := $(CRTEND_O) $(BUILD)/boot/crtn.o
CXXFLAGS ?= -std=gnu++14 \
			-ffreestanding \
			-fno-exceptions \
			-fno-rtti \
			-Wall -Wextra \
			-fno-pic \
			-fno-pie \
			-fno-stack-protector
CPPFLAGS += -I kernel/include 
LDFLAGS += -static
DEBUG = -g

KERNELFILES = $(BUILD)/kernel/start.o \
				$(BUILD)/kernel/kernel.o \
				$(BUILD)/kernel/io.o \
				$(BUILD)/kernel/console.o \
				$(BUILD)/kernel/log.o \
				$(BUILD)/kernel/assert.o \
				$(BUILD)/kernel/debug.o \
				$(BUILD)/kernel/global.o \
				$(BUILD)/kernel/schdule.o \
				$(BUILD)/kernel/task.o \
				$(BUILD)/kernel/handler.o \
				$(BUILD)/kernel/interrupt.o \
				$(BUILD)/kernel/clock.o \
				$(BUILD)/kernel/time.o \
				$(BUILD)/kernel/rtc.o \
				$(BUILD)/kernel/memory.o \
				$(BUILD)/kernel/gate.o \
				$(BUILD)/lib/string.o \
				$(BUILD)/lib/stdlib.o \
				$(BUILD)/lib/bitmap.o \
				$(BUILD)/lib/syscall.o \

QEMU = qemu-system-i386 \
	-m 32M \
	-boot c \
	-drive file=$<,if=ide,index=0,media=disk,format=raw \
	-audiodev pa,id=hda \
	-machine pcspk-audiodev=hda \
	-rtc base=localtime \

all: $(BUILD)/master.img $(BUILD)/system.map

$(BUILD)/%.bin: $(SRC)/%.asm
	@mkdir -p $(dir $@)
	nasm -f bin $(DEBUG) $< -o $@

$(BUILD)/%.o: $(SRC)/%.asm
	@mkdir -p $(dir $@)
	nasm -f elf32 $(DEBUG) $< -o $@

$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) $(DEBUG) -c $< -o $@

$(BUILD)/kernel.bin:    $(BUILD)/boot/crti.o \
						$(BUILD)/boot/crtn.o \
						$(KERNELFILES)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $(KERNELFILES) -o $@ -Ttext $(ENTRY)

# 提前对 elf 文件进行处理
$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy -O binary $< $@

# 生成 kernel 的符号表
$(BUILD)/system.map: $(BUILD)/kernel.bin
	nm $< | sort > $@

$(BUILD)/%.img: $(BUILD)/boot/boot.bin \
				$(BUILD)/boot/loader.bin \
				$(BUILD)/system.bin
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 seek=2 count=4 conv=notrunc
# 测试 system.bin 小于 100k，否则修改下面命令中的 count 的大小
	test -n "$$(find $(BUILD)/system.bin -size -100k)"
# 将内核二进制文件写入硬盘
	dd if=$(BUILD)/system.bin of=$@ bs=512 seek=10 count=200 conv=notrunc

bochs: $(BUILD)/master.img
	bochs -q

qemu: $(BUILD)/master.img
	$(QEMU)

qemug: $(BUILD)/master.img
	$(QEMU) -S -s

$(BUILD)/master.vmdk: $(BUILD)/master.img
	qemu-img convert -O vmdk $< $@

vmdk: $(BUILD)/master.vmdk

clean:
	rm -rf $(BUILD)

test: $(BUILD)/kernel.bin

.PHONY: clean bochs test vmdk
