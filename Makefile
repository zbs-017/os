SRC = kernel/src
BUILD = build

all: $(BUILD)/master.img

$(BUILD)/%.bin: $(SRC)/%.asm
	@mkdir -p $(dir $@)
	nasm -f bin $< -o $@

$(BUILD)/%.img: $(BUILD)/boot/boot.bin $(BUILD)/boot/loader.bin
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 seek=2 count=4 conv=notrunc

bochs: $(BUILD)/master.img
	bochs -q

clean:
	rm -rf $(BUILD)

.PHONY: clean bochs