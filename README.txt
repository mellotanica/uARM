UARM is a simple ARM7TDMI-based machine emulator

###### INSTALL #######

To compile uarm you need the following packages:

- qmake
- make
- qt v4.8.2
- gcc (g++)

To build the program run ./compile inside the main directory


###### RUNNING #######

Ihe executable is named uarm and is located insite the main
directory after build, it takes the standard qt parameters.

In the main window are shown the processor's and coprocessor's
registers as well as the actual loaded pipeline. To run a
program click on open button and select an assembled file
generated with the assemble script located in the main
directory.

Use the '+' and '-' buttons or the slider to set the emulation
speed (the speed can be modified during emulation).

The 'View Ram' button opens Ram Inspector windows, which let
you browse the actual memory contents: specify start address
in the left field and end address in the right field, then
click on 'Display Portion' button to view memory contents.


###### TESTING #######

In test directory is included a simple assembly program to
try the emulator.

First of all assemble the program with assemble script by
running '$:./assemble test/sample.s' inside the main directory.

Now launch the emulator and click on 'Open' button, browse
to test directory and select sample.bin file.

To begin the emulation click on 'Start' button.
