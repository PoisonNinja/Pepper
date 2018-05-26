# Pepper
Pepper contains the userspace programs and toolchain for the Quark kernel.

# Getting Started
Pepper, and by extension Quark, uses a custom GCC/Binutils based toolchain with the target triplet ${ARCH}-pepper. While Pepper in the past used the LLVM suite of tools, I found it to be lacking in sysroot support that made creating a true cross compiler hard. In addition, there is a lot of documentation for creating a cross compiler for GCC and binutils, while LLVM is a little more lacking.

Pepper also utilizes the newlib C library. It is also patched with Pepper specific code and is still limited in functionality given the limited support by Quark.

If you are using relatively new versions of MacOS or Ubuntu, the build process is largely automated. Other distributions users will be guided through obtaining the prerequisites, and the build script can still build the actual toolchain for you.

## Prerequisites
All the necessary prerequisites are installed by the build script. Be aware that on MacOS, the script will install Homebrew. If you do not want that, you will need to install the required packages yourself.

## Building the toolchain
To get started, run `toolchain.sh` in the `toolchain/` directory:
```
toolchain/toolchain.sh
```

The script by default will install dependencies and build everything. If you do not want that, pass the various --skip-* options. To see those flags and other options, run toolchain.sh with the --flag switch.

The script will detect what operating system you are running on and prompt you for confirmation. If its guess is right, select `y` to continue. Otherwise, hit `n`.

If you hit `y`, the script will install the prerequisites. If you hit `n`, the script will print out a list of packages to install and wait for you to indicate that you have done so.

Then, the script will download, extract, and patch the GCC, binutils, and newlib source code into a temporary directory. If the script is cancelled at any time, it will automatically clean up after itself and delete the temporary directories.

The script will ensure that all the build files are setup correctly and start the build. This will take a while. The toolchain will be installed under `toolchain/local`, so you can simply delete that folder if the toolchain ever breaks or you don't want it anymore.

Once it's done, the script will prompt you to setup the environment variables. This will only need to be done once so CMake can know where our custom compiler is located. For future runs, you can directly start building without needing to perform the environment setup.
```
. build/envsetup.sh
```

If you are on MacOS, the script will ask you to build GRUB. This process is also automated. Simply run `grub.sh` in the `toolchain/` directory:
```
toolchain/grub.sh
```

GRUB requires objconv, so this script will automatically download objconv, build it, and remove it once the script finishes. GRUB will also be installed under `toolchain/local`.

## Obtaining the source
You will need to obtain the kernel source. It is stored as a git submodule, so simply run:
```
git submodule update --init --recursive
```

It is likely that the revision of quark downloaded will be an older version, so `cd` to the quark directory and run:
```
git checkout master
```

As for the userspace, the code is included in this repository, so no further action is necessary.

# Build preparation
Before we start building, we need to set up the build scripts. As Pepper and Quark use CMake, you will need to use cmake to generate the build files. You have two options: first choice is to use the automated script to set up everything for you. If that doesn't work or you prefer to do it manually, use the second choice.

## Automatic
We will be using configure.sh under build/ to do set up everything. For default settings:
```
build/configure.sh --architecture=<your architecture> --no-newlib
```

It will run and set up a couple things. Once it's done, you're ready to build.

configure.sh also allows setting custom CMake arguments for both the kernel and userspace. By default, the kernel CMake run uses `-G Ninja`. To use a different argument to CMake:
```
build/configure.sh --architecture=<your architecture> --kernel_args="<your kernel arguments>" --no-newlib
```

For userspace arguments:
```
build/configure.sh --architecture=<your architecture> --user_args="<your user arguments>" --no-newlib
```

To see the entire list of arguments that configure.sh supports, run it with the --help flag.

## Manual
Before you start, make sure that you have set up the environment using `. build/envsetup.sh`. This ensures that CMake can find the cross compiler and only needs to be run this time. In the future it can be omitted.

You will probably want to read Quark's README for instructions on setting up the build system for Quark itself. By the end of following the instructions, you should have a build/ directory under quark/ with all the build files set up.

For userspace, create a directory called `build` under userspace, `cd` into it, and run
```
cmake .. -G <your favorite build generator>
```

After this, you should be good to go.

# Building
Once everything is set up , simply run `make` in the Pepper directory to build everything.

To launch QEMU, run `make qemu`. For remote debugging using GDB, run `make remote`. A plethora of other options are available, and they can be listed using `make help`. Any unrecognized options are automatically passed through to Quark, so there is no need to `cd` to Quark to run a build command.
