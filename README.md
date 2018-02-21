# Pepper
Pepper contains the userspace programs and toolchain for the Quark kernel.

# Getting Started
Pepper, and by extension Quark, use the LLVM toolchain instead of the more traditional GCC and Binutils toolchain. While in theory LLVM is cross-compile ready OOB, I found that on most systems (Mac, Ubuntu) the preinstalled toolchain is either broken (Mac) or missing necessary tools (Ubuntu). So, until most operating systems can ship a _full_ LLVM toolchain, we will need to build our own.

If you are using relatively new versions of MacOS or Ubuntu, the build process is largely automated. Other distributions users will be guided through the prerequisites, and the build script can still build LLVM for you.

## Prerequisites
All the necessary prerequisites are installed by the build script. Be aware that on MacOS, the script will install Homebrew. If you do not want that, you will need to install the required packages yourself.

## Building the toolchain
To get started, run `toolchain.sh` in the `toolchain/` directory:
```
toolchain/toolchain.sh
```

The script will detect what operating system you are running on and prompt you for confirmation. If its guess is right, select `y` to continue. Otherwise, hit `n`.

If you hit `y`, the script will install the prerequisites. If you hit `n`, the script will print out a list of packages to install and wait for you to indicate that you have done so.

Then, the script will download the entire LLVM, Clang, and ld.lld source code into a temporary directory. If the script is cancelled at any time, it will automatically clean up after itself and delete the temporary directories.

The script will ensure that all the build files are setup correctly and start the build. This will take awhile. The toolchain will be installed under `toolchain/local`, so you can simply delete that folder if the toolchain ever breaks or you don't want it anymore.

Once it's done, the script will prompt you to setup the environment variables. This will only need to be done once so CMake can know where our custom compiler is located. For future runs, you can directly start building without needing to perform the environment setup.

If you are on MacOS, the script will ask you to build GRUB. This process is also automated. Simply run `grub.sh` in the `toolchain/` directory:
```
toolchain/grub.sh
```

Unfortunately, GRUB doesn't build with Clang on Macs, so GRUB will download and build a GCC/Binutils cross-compiling toolchain. It will also download and install objconv. These programs and files will be removed after GRUB is built. GRUB will also be installed under `toolchain/local`.

Once GRUB is built, the system is configured.

## Obtaining the kernel source
You will need to obtain the kernel source. It is stored as a git submodule, so simply run:
```
git submodule update --init --recursive
```

It is likely that the revision of quark downloaded will be an older version, so `cd` to the quark directory and run:
```
git checkout master
```

# Building
You will probably want to read Quark's README for instructions on setting up the build system for Quark itself.

Once everything is set up for Quark, simply run `make` in the Pepper directory to build everything.

To launch QEMU, run `make qemu`. For remote debugging using GDB, run `make remote`. A plethora of other options are available, and they can be listed using `make help`. Any unrecognized options are automatically passed through to Quark, so there is no need to `cd` to Quark to run a build command.
