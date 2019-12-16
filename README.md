# rtems-pru
Application for developing/testing the pru-drivers on rtems.

## Building all dependencies
1. A rtems installation is needed.
Please take a look at the rtems Quickstart-Guide on this.
https://github.com/nilhoel1/rtems-libbsd/commits/pruss
If the whole libbsd output, I used to analyse the libbsd init process 
is needed use this branch:
https://docs.rtems.org/branches/master/user/start/index.html


2. It is necessary to use my current rtems-libbsd Repository.
The pru drivers are not committed yet.
So please install rtems-libbsd from here:\
https://github.com/nilhoel1/rtems-libbsd/tree/ti_pruss

3. If a sd card image with U-Boot is needed, also install rtems-tools. And use ```rtems-boot-image```.
 https://github.com/RTEMS/rtems-tools

## Building the Application
The Application can be build with the following commands.
All commands are run from repository tree.

1. Configure the waf file and point it to the tools, rtems, libbsd and choose your bsp:
```
./waf configure \
--rtems-tools=$HOME/development/rtems/5 \
--rtems=$HOME/development/rtems/5 \ with uboot
--rtems-libbsd=$HOME/development/rtems/rtems-libbsd \
--rtems-bsps=arm/beagleboneblack
```

2. Build the app:
```
./waf
```

Now the executable can be found in the build folder.

## Creating the Image
A Image can be created with this command when rtems-tools are isntalled.
```
$ rtems-boot-image --help
usage: rtems-boot-image [-h] [-l LOG] [-v] [-s IMAGE_SIZE] [-F FS_FORMAT]
                        [-S FS_SIZE] [-A FS_ALIGN] [-k KERNEL] [-d FDT]
                        [-f FILE] [--net-boot] [--net-boot-dhcp]
                        [--net-boot-ip NET_BOOT_IP]
                        [--net-boot-server NET_BOOT_SERVER]
                        [--net-boot-file NET_BOOT_FILE]
                        [--net-boot-fdt NET_BOOT_FDT] [-U CUSTOM_UENV]
                        [-b BOARD] [--convert-kernel] [--build BUILD]
                        [--no-clean] -o OUTPUT
                        paths [paths ...]

Provide one path to a u-boot build or provide two paths to the built the first
and second stage loaders, for example a first stage loader is 'MLO' and a
second 'u-boot.img'. If converting a kernel only provide the executable's
path.

positional arguments:
  paths                 files or paths, the number and type sets the mode.

optional arguments:
  -h, --help            show this help message and exit
  -l LOG, --log LOG     log file (default: rtems-log-boot-image.txt).
  -v, --trace           enable trace logging for debugging.
  -s IMAGE_SIZE, --image-size IMAGE_SIZE
                        image size in mega-bytes (default: 64m).
  -F FS_FORMAT, --fs-format FS_FORMAT
                        root file system format (default: fat16).
  -S FS_SIZE, --fs-size FS_SIZE
                        root file system size in SI units (default: auto).
  -A FS_ALIGN, --fs-align FS_ALIGN
                        root file system alignment in SI units (default: 1m).
  -k KERNEL, --kernel KERNEL
                        install the kernel (default: None).
  -d FDT, --fdt FDT     Flat device tree source/blob (default: None).
  -f FILE, --file FILE  install the file (default: None).
  --net-boot            configure a network boot using TFTP (default: False).
  --net-boot-dhcp       network boot using dhcp (default: False).
  --net-boot-ip NET_BOOT_IP
                        network boot IP address (default: None).
  --net-boot-server NET_BOOT_SERVER
                        network boot server IP address (default: None).
  --net-boot-file NET_BOOT_FILE
                        network boot file (default: 'rtems.img').
  --net-boot-fdt NET_BOOT_FDT
                        network boot load a fdt file (default: None).
  -U CUSTOM_UENV, --custom-uenv CUSTOM_UENV
                        install the custom uEnv.txt file (default: None).
  -b BOARD, --board BOARD
                        name of the board (default: 'list').
  --convert-kernel      convert a kernel to a bootoader image (default:
                        False).
  --build BUILD         set the build directory (default: 'ribuild').
  --no-clean            do not clean when finished (default: True).
  -o OUTPUT, --output OUTPUT
                        image output file name

```
Create the image with first and second stage boot loader.

Flash an image with :
```
dd if=PATH_TO_SD-IMAGE of=PATH_TO_SD_CARD bs=1M
```
Also tools like etcher can be used. 

Now mount the sd card and copy the content of the ```image``` folder to it and the pru.exe.img file.
The other content in the image folder contains a device tree binary, a device tree overlay, and the uEnv.txt for U-Boot.
When the app is rebuild it is only required to overwrite the pru.exe.img on the sd card with the one in the build folder.

## Using the APP
### The current app is having issues opening files,  see workaround.
When all dependencies are installed and the app builded well, than after booting you should find the device ```dev/pruss0``` and the sd card should be mounted in ```/media```. I am currently working on a shell program to load pru code and execute it. When one is provided it is necessary to map the pru interrupts to IRQ with:
```
sysctl dev.ti_pruss.0.irq.2.channel=2 # mapping channel 2 to irq2
sysctl dev.ti_pruss.0.irq.2.event=16 # mapping event 16 to irq2
sysctl dev.ti_pruss.0.irq.2.enable=1 # enable the irq2
sysctl dev.ti_pruss.0.global_interrupt_enable=1 # enable PRU global interrupts
```
This should create the device  ```/dev/pruss0.irqN```, where N is the interrupt channel and with the above example this should be 2. When runing test.bin from the pruexamples a interrupt should be triggered.

The pru example test.p can then be loaded to pru with:
```
# -p 0 is pru0, -t ti is type Texas Instruments, -er means enable+reset
pructl -p 0  -t ti -er test.bin
```
But make sure to be in the same directoire as the test.bin.

## Workaround 
In this branch the above chapter wont work.
I am having troubles with the file system.
Therefore programs loading the pruss code from header files were added for testing.
So please just run:
```
testpru
```
In the shell, to run the loop file from pruexamples folder.

To run the example showing that IRQ interupts work please apply the afore mentioned  commands and run:
```
testirq
```

## Pruss shell command
Also included in this repo is the pruss command.
It can be used by simply typing.
```
pruss <command> <options>
```
The help output looks like follows:
```
SHLL [/] # pruss -h
pruss: PRU Loader
  pruss [-hl] <command>
   where:
     command: A Action for the PRU. See -l for a list plus help.
     -h:      This help
     -l:      The command list.
```
And the commands are:
```
SHLL [/] # pruss -l
pruss: commands are:
  init          Allocates and resets PRU
  alloc         Allocated the PRU memory
  reset         Resets the PRU, reset -[01]
  upload        Uploads file to PRU, upload -[01] <file>
  start         Starts the PRU, start -[01]
  step          Executes singe command on PRU, step -[01]
  stop          Stops the PRU, stop -[01]
  wait          Starts the PRU and waits for completion, wait -[01]
  free          Frees the PRU Object memory
```

## Compiling your own PRU code
To compile the PRU code examples I used the pasm pru assembler.
It can be installed on Linux by cloning this Repo: \
https://github.com/beagleboard/am335x_pru_package\
And install the pasm.
```
PRU Assembler Version 0.87
Copyright (C) 2005-2013 by Texas Instruments Inc.

Usage: pasm [-V#EBbcmLldfz] [-Idir] [-Dname=value] [-Cname] InFile [OutFileBase]

    V# - Specify core version (V0,V1,V2,V3). (Default is V1)
    E  - Assemble for big endian core
    B  - Create big endian binary output (*.bib)
    b  - Create little endian binary output (*.bin)
    c  - Create 'C array' binary output (*_bin.h)
    m  - Create 'image' binary output (*.img)
    L  - Create annotated source file style listing (*.txt)
    l  - Create raw listing file (*.lst)
    d  - Create pView debug file (*.dbg)
    f  - Create 'FreeBasic array' binary output (*.bi)
    z  - Enable debug messages
    I  - Add the directory dir to search path for 
         #include <filename> type of directives (where 
         angled brackets are used instead of quotes).

    D  - Set equate 'name' to 1 using '-Dname', or to any
         value using '-Dname=value'
    C  - Name the C array in 'C array' binary output
         to 'name' using '-Cname'

```

PRU code can also be produced with Texas Instruments tools and gcc 10 should also support TI-PRU compilation.

### GSoC 2019 coding Period ends August 19th 2019.
