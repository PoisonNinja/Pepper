# Pepper
Pepper is an operating system based on the [Quark](https://github.com/PoisonNinja/quark) kernel. This repository contains the various programs that comprise the operating system and the tools needed to build them.

# Getting Started
Obtaining and building the prerequisites for Pepper is a largely automated process, as long as you are on a supported system. I personally test on MacOS (>= 10.13), Ubuntu (>= 18.04), and WSL (>= 1809, Ubuntu image), so these platforms are the best supported. In theory any UNIX-like operating system should work but will require more work to set up properly.

On MacOS and Ubuntu-derivatives, the build script will automatically install all needed prerequisites. On other operating systems/distros, the build script will guide you through obtaining the needed prerequisites.

Pepper, and by extension Quark, uses a custom GCC/Binutils based toolchain with the target triplet ${ARCH}-quark.

Pepper also utilizes the newlib C library. It is heavily patched with Pepper specific code and can be found in a [separate repository](https://github.com/PoisonNinja/newlib).

Regardless of whether you are on MacOS/Ubuntu or some other system, the build script will handle building and installing the toolchain and C library for you.

## Prerequisites
As mentioned before, all the necessary prerequisites are installed by the build script if you are on MacOS or an Ubuntu-derivative.

On other platforms, you will need to install the prerequisites yourself.

Be aware that on MacOS, the script will install Homebrew. If you do not want that, you will need to install the required packages yourself.

Currently, the prerequisites are:

| Name | Description|
|------|------------|
|build-essential | Includes GCC and binutils for the host. Needed to build the cross-compiler |
| cmake | The build system used in Quark to generate the build files |
| genext2fs | Used to generate the ext2 hard disk image |
| grub-common | Pepper uses GRUB to boot, and this package installs the tools needed to install it. |
| libmpfr-dev | A GCC/Binutils dependency |
| libmpc-dev | A GCC/Binutils dependency |
| libgmp3-dev | A GCC/Binutils dependency |
| nasm | The assembler used in Quark |
| ninja-build | The build tool used in Quark. It's faster than Make but is intended to be generated by machines, hence the use of CMake  |
| qemu | Emulator. You could replace this with Bochs, Virtualbox, or some other emulator |
| texinfo | Needed to build documentation for GCC and some other stuff |
| xorriso | Generate an ISO image |
| curl | Used to download the prerequisites |

Some additional packages that are nice to have and are automatically installed by the build script are:

| Name | Description|
|------|------------|
| clang-format | Format the code in a standard way. Easy way for me to enforce a coding style |

## Building the toolchain
To get started, run `toolchain.sh` in the `toolchain/` directory:
```
toolchain/toolchain.sh
```

The build script supports several options that control its behavior. The most interesting flags are `--arch=<arch>`, which allows you to build a toolchain for different architectures, and `--local-*`, which allows you to use a local copy of GCC, binutils, or newlib.

There are also various `--skip-*` flags. These allow you to skip various parts of the toolchain build process, but these are generally not very useful unless you are testing something specific.

For a complete list of all the flags, run `toolchain.sh` with the `--help` flag.

The script will first detect what operating system you are running on and prompt you for confirmation. If its guess is right, select `y` to continue. Otherwise, hit `n`.

If you hit `y` and you are on a supported system, the script will install the prerequisites for you. If you hit `n` or are using an unsupported system, the script will print out a list of packages to install and wait for you to indicate that you have done so.

Then, the build script will download, extract, and patch the GCC, binutils, and newlib source code into a temporary directory. If the script is cancelled at any time, it will automatically clean up after itself and delete the temporary directories.

The script will ensure that all the build files are setup correctly and start the build. This will take a while. The toolchain will be installed under `toolchain/local`, so you can simply delete that folder if the toolchain ever breaks or you don't want it anymore.

Once it's done, the script will prompt you to setup the environment variables. This will only need to be done once so CMake can know where our custom compiler is located. For future runs, you can directly start building without needing to perform the environment setup. Simply run the following command:
```
. build/envsetup.sh
```

If you are on MacOS, the script will also ask you to build GRUB. This process is also automated. Simply run `grub.sh` in the `toolchain/` directory:
```
toolchain/grub.sh
```

GRUB requires objconv, so this script will automatically download objconv, build it, and remove it once the script finishes. GRUB will also be installed under `toolchain/local`.

## Obtaining the source
You will need to obtain the kernel source. Quark is stored as a git submodule, so simply run:
```
git submodule update --init --recursive
```

It is likely that the revision of Quark downloaded will be an older version, so `cd` to the Quark directory and run:
```
git checkout master
```

As for the userspace, the code is included in this repository, so no further action is necessary.

# Build System
Before we start building, we need to set up the build system. As Pepper and Quark use CMake, you will need to use Cmake to generate the build files.

We will be using `configure.sh` under build/ to do set up everything. For sane default settings:
```
build/configure.sh --no-newlib
```

By default, this will configure the build system to target x86_64. If this is not your intended architecture, append the `--arch=<your architecture>` flag.

Since we just built newlib, there's no need to build newlib again. If you're switching architectures, sometimes it's useful to remove the `--no-newlib` flag.

`configure.sh` will run and set up a couple things. Once it's done, you're ready to build.

## Options
`configure.sh` allows setting custom CMake arguments for both the kernel and userspace. This will allow you to tweak the build generator, build configuration, and many other settings.

To pass a flag to the kernel CMake, use the `--kernel_args` flag:
```
build/configure.sh --kernel_args="<your kernel arguments>" --no-newlib
```

For example, to enable a debug build, add `-DCMAKE_BUILD_TYPE=Debug`, so that the call to `configure.sh` looks like:
```
build/configure.sh --kernel_args="-DCMAKE_BUILD_TYPE=Debug" --no-newlib
```

By default, `configure.sh` uses the Ninja build system. If you would like to use a different build system, such as Make, append `-G <your generator>` to `--kernel_args`.

For userspace arguments, use `--user_args`:
```
build/configure.sh --user_args="<your user arguments>" --no-newlib
```

To see the entire list of arguments that `configure.sh` supports, run it with the --help flag.

# Building
Once everything is set up , simply run `make` in the Pepper directory to build everything. The default target generates an ext2 hard disk image and a bootable ISO.

You can also just build specific components. For example, to build just the ISO, run `make pepper.iso`. Some other interesting targets are:
* `disk.img` - ext2 hard disk image
* `initrd`
* `modules` - Build ALL kernel modules and install them
* `userspace` - Build ALL userspace programs and install them

To clean all files, run `make clean`. This will also clean all Quark build files.

To see all build options, run `make help`. Any unrecognized options will be passed through to Quark.

# Running Pepper
Since Pepper/Quark is in an early stage, it's probably best not to test on real hardware. I personally use QEMU, but feel free to other emulators. Most likely you will encounter new bugs when booting on a new emulator/hardware. If that happens, please file a Github issue.

The generated bootable ISO contains GRUB, the kernel, and the initrd. The ext2 disk image contains the rest of the userspace.

To launch QEMU with sane defaults, run `make qemu`.

For remote debugging using GDB, run `make remote`. This launches QEMU but waits for GDB to connect before continuing boot, allowing you to debug Quark as if it were a regular program.

If you want to test on other emulators, make sure to set the ext2 disk image as the first hard disk and to insert the bootable ISO as a disk.

# Usage
There's not really much to do right now as a lot of work is ongoing in Quark on behind-the-scene things (such as  memory management, process management, etc.).

Currently, Pepper boots into a simple shell. It interprets all input as a path to a program and tries to execute it. That's literally it.

As I add more stuff, hopefully this will become more populated in the future :)

# Hacking
As the source code is constantly evolving, I haven't really found any time to write documentation (it would be out of date soon anyways). I salute you for braving my code :P

In all seriousness though, the code should be reasonably organized and easy to follow. For Quark-specific hacking, see the README there.

The source code organization is as follows:
* build/ - Build scripts
* modules/ - Kernel modules
    * ahci/ - AHCI disk driver
    * test/ - A sample kernel module. You can use this as a reference for writing other kernel modules
* quark/ - The actual kernel
* sysroot/ - Will be turned into the hard disk image. Mostly dynamically generated except for a few specific configuration items
* toolchain/ - Patches for the toolchain and scripts for building the toolchain. Also stores the actual toolchain once it's built.
* userspace/ - Userspace programs
    * core/ - Core files such as init, sh, and terminal
    * extra/ - Other programs that aren't as important
    * initrd/ - Programs that are intended to be stored in the initrd
    * lib/ - Libraries

The included .clang-format file should ensure that clang-format formats all the code in a standard way. Please use it.

All changes should be submitted via pull requests here on Github.

# Contact
The best way to reach me about Pepper issues is to simply file a Github issue. Otherwise, you can find the latest contact information on my [website](https://poison.ninja).
