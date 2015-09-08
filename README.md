# JackOS
A Simple operating system (Work in progress)

To create a new floppy image:
make floppy

To create a new kernel:
make

To execute on a virtual machine:
qemu-system-i386 -kernel kernel <-monitor stdio> <-s -S> <-hda floppy.img>
