UARM is a simple ARM7TDMI-based machine emulator

###### INSTALL #######

To compile uarm you need the following packages:

- qmake
- make
- qt v4.8.2
- gcc (g++)

To build the program run ./compile inside the main directory.

You will also need a cross-compile toolchain to build
executables for ARM architecture.

If you are running a Debian-based machine you can try
Emdebian (https://wiki.debian.org/EmdebianToolchain), anyways
any ARM cross-compiling toolchain is ok.

###### RUNNING #######

Ihe executable is named uarm and is located inside the main
directory after running ./compile; it takes the standard qt
parameters as input.

The main window shows the processor's and coprocessor's
registers as well as the actual loaded pipeline. To run a
program click on the config button and select the converted
core file in the appropriate field.

Use the '+' and '-' buttons or the slider to set the emulation
speed (the speed can be modified during emulation).

The 'View Ram' button opens Ram Inspector windows, which let
you browse the actual memory contents: specify start address
in the left field and end address in the right field, then
click on 'Display Portion' button to view memory contents.

The configuration value "Default Clock Rate" modifies the
emulated processor speed (NOT the same as emulation speed),
with higher values the devices will react slower because
the processor is faster but device execution speed does not
change.

###### TESTING #######

In test directory is included a simple hello world program to
try the emulator, "helloWorld.c".

First of all you need to compile the program into an object
file with your cross-compiler (the commands issued from here on
are relative to Emdebian toolchain, with a different one the
syntax may change a little):

$> cd test
$> arm-linux-gnueabi-gcc -mcpu=arm7tdmi -c -o helloWorld.o \
   helloWorld.c

This command builds an object file "helloWorld.o" compiled
for ARM7TDMI processor.

Now you need to link the file against the system libraries:

$> cd ..
$> arm-linux-gnueabi-ld -T ldscript/elf32ltsarm.h.uarmcore.x \
   -o test/helloWorld.elf \
   facilities/crtso.o facilities/libuarm.o test/helloWorld.o

The "elf32ltsarm.h.uarmcore.x" file is a specific linker script
for linking uARM executables.

The last step remaining is to convert "helloWorld.elf" into
a uARM core.uarm executable file:

$> ./elf2uarm -k test/helloWorld.elf

The last command with -k flag created a core file inside the
same directory as the original elf file wich can be executed
from uARM virtual machine.

It is time to run the program itself:

$> ./uarm

Search the top bar for the config button (the first one on
the left) and choose the core file you just created.

Save the configurations by clicking "Ok" and reset the machine
with the reset button.

Press the play button to run the machine at full speed or set
emulation speed through the central slider in the top bar.

To make the terminal window show up click on "Terminals" button
and choose the desired terminal, you can also press Alt+[n]
where n is the number of the desired terminal.
