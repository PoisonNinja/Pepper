.PHONY: qemu

SHELL := /bin/bash
OS := $(shell uname -s)

QEMU := qemu-system-x86_64

QEMU_ARGS :=-m 1024 -rtc base=localtime -cdrom bootable.iso -no-reboot -no-shutdown -cpu Nehalem
QEMU_AHCI := -drive file=hdd.img,if=none,id=hdd,format=raw -device ich9-ahci,id=ahci -device ide-drive,drive=hdd,bus=ahci.0
QEMU_ACCEL := -M accel=kvm:tcg
QEMU_SERIAL := -serial stdio
QEMU_MONITOR := -monitor stdio
QEMU_REMOTE := -s -S

TOOLCHAIN_PREFIX := toolchain/local/bin

ifeq ($(OS),Darwin)
	GRUB_MKRESCUE := $(TOOLCHAIN_PREFIX)/grub-mkrescue
else
	GRUB_MKRESCUE := grub-mkrescue
endif

HDD := $(shell find hdd/)

# Userfacing targets
all: bootable.iso hdd.img

bootable.iso: hdd/boot/quark.kernel
	$(GRUB_MKRESCUE) -o bootable.iso hdd

clean:
	$(RM) bootable.iso hdd.img hdd/boot/quark.kernel
	@cmake --build quark/build --target clean

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
	@echo "Unrecognized options will be automatically passed through to Quark"
	@echo "Therefore, you can run kernel makefile targets from this directory"

kernel: hdd/boot/quark.kernel

remote: bootable.iso hdd.img
	@$(QEMU) $(QEMU_ARGS) $(QEMU_AHCI) $(QEMU_ACCEL) $(QEMU_SERIAL) $(QEMU_REMOTE)

qemu: bootable.iso hdd.img
	@$(QEMU) $(QEMU_ARGS) $(QEMU_AHCI) $(QEMU_ACCEL) $(QEMU_SERIAL)

monitor: bootable.iso hdd.img
	@$(QEMU) $(QEMU_ARGS) $(QEMU_AHCI) $(QEMU_ACCEL) $(QEMU_MONITOR)

# Internal targets
hdd/boot/quark.kernel: FORCE
	@cmake --build quark/build --target install

FORCE:

# Pass unrecognized Makefile targets to the Quark build process
# Allows us to run stuff like make config in this directory
.DEFAULT:
	@echo "Pepper does not recognize this target. Passing to Quark..."
	@echo
	@cmake --build quark/build --target $(MAKECMDGOALS)
