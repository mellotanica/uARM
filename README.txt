UARM is a simple ARM7TDMI-based machine emulator

###### INSTALL #######

To compile uarm you need the following packages:

- qmake (qt5-qmake)
- make
- qt >= v5.3.0 (including declarative)
- gcc (g++)
- libelf1
- libelf-dev
- libboost-dev

To build the program run ./compile inside the main directory.

To install the program in the default system directories run
./install.sh inside the main directory.

To install the program in a custom directory run ./install.sh -d [path]
where path is the absolute path to the custom directory 
(e.g. ./install.sh -d /usr/local).

You will also need a cross-compile toolchain to build
executables for ARM architecture.

If you are running a Debian-based machine you can try
gcc-arm-none-eabi (https://launchpad.net/gcc-arm-embedded/+download)
or Emdebian (https://wiki.debian.org/EmdebianToolchain), anyways
any ARM cross-compiling toolchain supporting ARMv4 is ok.

###### RUNNING #######

Ihe executable is named uarm and is located inside the main
directory after running ./compile and in /usr/bin after
installation; it takes the standard qt parameters as input.

The main window shows the processor's and coprocessor's
registers as well as the actual loaded pipeline. To run a
program click on the config button and select the converted
core file in the appropriate field.

Before running anything you must power on the machine by
clicking the appropriate button. You need to restart the machine
each time you change the kernel or bios files.

Use the '+' and '-' buttons or the slider to set the emulation
speed (the speed can be modified during emulation).

The 'View Ram' button opens Ram Inspector windows, which let
you browse the actual memory contents: specify start address
in the left field and end address in the right field, then
click on 'Display Portion' button to view memory contents.

The 'Terminals' button let you open the enabled terminals'
windows. You can do that also with the shortcut Alt+[n]
where [n] is the number of the terminal you want to view.

The configuration value "Default Clock Rate" modifies the
emulated processor speed (NOT the same as emulation speed),
with higher values the devices will react slower because
the processor is faster but device execution speed does not
change.

###### TESTING #######

In test directory is included a simple hello world program to
try the emulator, "helloWorld.c". After installationsuch file
will also be located in /usr/include/uarm/test directory.

From now on it will be assumed that you installed the program
with install.sh script, also the commands issued are relative
to none-eabi toolchain.

First of all you need to get the test program and compile it
into an object file with your cross-compiler:

$> cp /usr/share/doc/uarm/examples/helloWorld.c .
$> arm-none-eabi-gcc -mcpu=arm7tdmi -c -o helloWorld.o \
   helloWorld.c

This command builds an object file "helloWorld.o" compiled
for ARM7TDMI processor.

Now you need to link the file against the system libraries:

$> arm-none-eabi-ld \
   -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x \
   -o helloWorld /usr/include/uarm/crtso.o \
   /usr/include/uarm/libuarm.o helloWorld.o

The "elf32ltsarm.h.uarmcore.x" file is a specific linker script
for linking uARM executables.

The last step remaining is to convert "helloWorld.elf" into
a uARM core.uarm executable file:

$> elf2uarm -k helloWorld

The last command with -k flag created a core file inside the
same directory as the original elf file wich can be executed
from uARM virtual machine.

It is time to run the program itself:

$> uarm

Search the top bar for the config button (the first one on
the left) and choose the core file you just created
(helloWorld.core.uarm).

Save the configurations by clicking "Ok" and power on/reset
the machine with the appropriate button.

Press the play button to run the machine at full speed or set
emulation speed through the central slider in the top bar.

If you want to view the code in action, bring up Terminal 0
bu clicking 'Terminals' button then choose Terminal 0 or simply
by pressing Alt+0.

###### NOTES #######

If you are going to install Emdebian toolchain be aware that
the last working version of the toolchain released is based on
Debian Squeeze, so if you are running a newer version you need
to add squeeze's main/contrib/non-free repository to your sources
list in addition to emdebian's one.

###### UNINSTALL ######

To uninstall the program use ./uninstall.sh script. 
It works as install.sh script, -d option lets you specify a
custom directory (selected during installation) to remove the 
program from.

The installation script creates a file in this directory
named custom_install_directory containing the path to the
custom installation if -d option was used during installation.
