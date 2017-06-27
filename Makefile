.PHONY: qemu

SHELL := /bin/bash

QEMU := qemu-system-x86_64

QEMU_ARGS := -serial stdio -m 2048 -rtc base=localtime -cdrom bootable.iso -no-reboot -no-shutdown -cpu Nehalem
QEMU_AHCI := -drive file=hdd.img,if=none,id=hdd,format=raw -device ich9-ahci,id=ahci -device ide-drive,drive=hdd,bus=ahci.0
QEMU_ACCEL := -M accel=kvm:tcg

HDD := $(shell find hdd/)

# Userfacing targets
all: bootable.iso hdd.img

bootable.iso: hdd/boot/mint.kernel
	grub-mkrescue -o bootable.iso hdd

clean:
	$(RM) bootable.iso hdd.img hdd/boot/mint.kernel
	@cmake --build mint/build --target clean

hdd.img: $(HDD)
	@echo
	@echo Generating hard disk image...
	@echo
	@genext2fs -d hdd -b 8192 hdd.img

help:
	@echo "======= Pepper build system help ======"
	@echo "all (default): Build kernel and bootable.iso"
	@echo "bootable.iso: Build bootable.iso. Also builds kernel"
	@echo "clean: Cleans *ALL* build files"
	@echo "help: Displays this message"
	@echo "kernel: Build the kernel"
	@echo "qemu: Builds everything and loads QEMU with bootable.iso inserted"
	@echo ""
	@echo "Unrecognized options will be automatically passed through to Mint"
	@echo "Therefore, you can run kernel makefile targets from this directory"

kernel: hdd/boot/mint.kernel

qemu: bootable.iso hdd.img
	@$(QEMU) $(QEMU_ARGS) $(QEMU_AHCI) $(QEMU_ACCEL)

# Internal targets
hdd/boot/mint.kernel: FORCE
	@cmake --build mint/build --target install

FORCE:

# Pass unrecognized Makefile targets to the Mint build process
# Allows us to run stuff like make config in this directory
.DEFAULT:
	@echo "Pepper does not recognize this target. Passing to Mint..."
	@echo
	@cmake --build mint/build --target $(MAKECMDGOALS)
