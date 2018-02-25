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

INITRD_EXCLUDE_LIST := \
	./hdd/boot \
	./hdd/usr

INITRD_EXCLUDE := $(addprefix "--exclude=", $(INITRD_EXCLUDE_LIST))

TOOLCHAIN_PREFIX := toolchain/local/bin

ifeq ($(OS),Darwin)
	GRUB_MKRESCUE := $(TOOLCHAIN_PREFIX)/grub-mkrescue
else
	GRUB_MKRESCUE := grub-mkrescue
endif

HDD := $(shell find hdd/ -path "hdd/boot/initrd.tar" -prune -o -print)

# Userfacing targets
all: bootable.iso hdd.img

bootable.iso: hdd/boot/quark.kernel hdd/boot/initrd.tar
	$(GRUB_MKRESCUE) -o bootable.iso hdd

clean:
	$(RM) bootable.iso hdd.img hdd/boot/quark.kernel hdd/boot/initrd.tar
	@cmake --build quark/build --target clean

hdd.img: $(HDD)
	@echo
	@echo Generating hard disk image...
	@echo
	@genext2fs -d hdd -b 65536 hdd.img

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

initrd: hdd/boot/initrd.tar

kernel: hdd/boot/quark.kernel

monitor: initrd bootable.iso hdd.img
	@$(QEMU) $(QEMU_ARGS) $(QEMU_AHCI) $(QEMU_ACCEL) $(QEMU_MONITOR)

remote: initrd bootable.iso hdd.img
	@$(QEMU) $(QEMU_ARGS) $(QEMU_AHCI) $(QEMU_ACCEL) $(QEMU_SERIAL) $(QEMU_REMOTE)

qemu: initrd bootable.iso hdd.img
	@$(QEMU) $(QEMU_ARGS) $(QEMU_AHCI) $(QEMU_ACCEL) $(QEMU_SERIAL)

userspace: FORCE
	@cmake --build userspace/build --target install

# Internal targets
hdd/boot/initrd.tar: userspace $(HDD)
	@echo
	@echo Generating initrd...
	@echo
	@tar -cvf hdd/boot/initrd.tar $(INITRD_EXCLUDE) hdd

hdd/boot/quark.kernel: FORCE
	@cmake --build quark/build --target install

FORCE:

# Pass unrecognized Makefile targets to the Quark build process
# Allows us to run stuff like make config in this directory
.DEFAULT:
	@echo "Pepper does not recognize this target. Passing to Quark..."
	@echo
	@cmake --build quark/build --target $(MAKECMDGOALS)
