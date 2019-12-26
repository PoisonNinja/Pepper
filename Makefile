.PHONY: qemu

include config.mk

SHELL := /bin/bash
OS := $(shell uname -s)

# Set from config.mk now
# ARCH ?= x86_64
QEMU := qemu-system-$(ARCH)

QEMU_ARGS :=-m 64 -rtc base=localtime -cdrom pepper.iso -no-reboot -no-shutdown -cpu Nehalem
QEMU_AHCI := -drive file=disk.img,if=none,id=hdd,format=raw -device ich9-ahci,id=ahci -device ide-hd,drive=hdd,bus=ahci.0
QEMU_ACCEL := -M accel=kvm:tcg
QEMU_DEBUG := -s
QEMU_BASE := $(QEMU_ARGS) $(QEMU_AHCI) $(QEMU_ACCEL) $(QEMU_DEBUG)
QEMU_SERIAL := -serial stdio
QEMU_MONITOR := -monitor stdio
QEMU_REMOTE := -S

TOOLCHAIN_PREFIX := toolchain/local/bin

# grub tools are installed locally on MacOS
ifeq ($(OS),Darwin)
	GRUB_MKRESCUE := $(TOOLCHAIN_PREFIX)/grub-mkrescue
else
	GRUB_MKRESCUE := grub-mkrescue
endif

INITRD := $(shell find initrd/ -print)
HDD := $(shell find sysroot/ -path "sysroot/boot/initrd.tar" -prune -o -print)

# Userfacing targets
all: pepper.iso disk.img

pepper.iso: sysroot/boot/quark.kernel sysroot/boot/initrd.tar
	$(GRUB_MKRESCUE) -o pepper.iso sysroot

clean:
	$(RM) pepper.iso disk.img sysroot/boot/quark.kernel sysroot/boot/initrd.tar
	@cmake --build quark/build --target clean
	@cmake --build userspace/build --target clean
	@cmake --build modules/build --target clean

disk.img: $(HDD)
	@echo
	@echo Generating hard disk image...
	@echo
# Modify this if genext2fs complains about not enough space
	@genext2fs -d sysroot -b 65536 disk.img

help:
	@echo "======= Pepper build system help ======"
	@echo "By default, Pepper assumes x86_64. To change architecture, edit the ARCH variable in config.mk"
	@echo "Supported architectures:"
	@echo " * x86_64"
	@echo "Current architecture: $(ARCH)"
	@echo
	@echo "all (default):   Build kernel and pepper.iso"
	@echo "pepper.iso:      Build pepper.iso. Also builds the kernel and initrd"
	@echo "clean:           Cleans *ALL* build files"
	@echo "help:            Displays this message"
	@echo "disk.img:        Generate the hard disk image"
	@echo "initrd:          Generate the initrd"
	@echo "kernel:          Build the kernel"
	@echo "monitor:         Same as QEMU, but loads monitor instead of serial output"
	@echo "remote:          Same as QEMU, but waits for GDB to conenct before starting"
	@echo "qemu:            Builds everything and loads QEMU with pepper.iso inserted"
	@echo "userspace:       Build and install the userspace"
	@echo ""
	@echo "Unrecognized options will be automatically passed through to Quark"
	@echo "Therefore, you can run kernel Makefile targets from this directory"

initrd: sysroot/boot/initrd.tar

kernel: sysroot/boot/quark.kernel

modules: FORCE
	@cmake --build modules/build --target install

monitor: initrd pepper.iso disk.img
	@$(QEMU) $(QEMU_BASE) $(QEMU_MONITOR)

remote: initrd pepper.iso disk.img
	@$(QEMU) $(QEMU_BASE) $(QEMU_SERIAL) $(QEMU_REMOTE)

qemu: initrd pepper.iso disk.img
	@$(QEMU) $(QEMU_BASE) $(QEMU_SERIAL)

userspace: FORCE
	@cmake --build userspace/build --target install

# Internal targets
sysroot/boot/initrd.tar: userspace modules $(INITRD)
	@echo
	@echo Generating initrd...
	@echo
	@tar -cvf sysroot/boot/initrd.tar -C initrd .

sysroot/boot/quark.kernel: FORCE
	@cmake --build quark/build --target install

FORCE:

# Pass unrecognized Makefile targets to the Quark build process
# Allows us to run stuff like make config in this directory
.DEFAULT:
	@echo "Pepper does not recognize this target. Passing to Quark..."
	@echo
	@cmake --build quark/build --target $(MAKECMDGOALS)
