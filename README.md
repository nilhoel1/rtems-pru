# rtems-pru
Application for developing/testing the pru-drivers on rtems.

## Building all dependencies
1. A rtems installation is needed.
Please take a look at the rtems Quickstart-Guide on this.
https://docs.rtems.org/branches/master/user/start/index.html


2. It is necessary to use my current rtems-libbsd Repository.
The pru drivers are not committed yet.
So please install rtems-libbsd from here:
https://github.com/nilhoel1/rtems-libbsd/tree/ti_pruss
3. If a sd card image is needed with uboot, also install rtems-tools.https://github.com/RTEMS/rtems-tools

## Building the Application
The Application can be build with the following commands.
All commands are run from repository tree.

1. Configure the waf file and point it to the tools, rtems, libbsd and choose your bsp:
```
./waf configure \
--rtems-tools=$HOME/development/rtems/5 \
--rtems=$HOME/development/rtems/5 \
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

Now mount the sd card and copy the content of the image folder to it and the pru.exe.img file.
When ever the app is rebuild it is only required to overwrite the pru.exe.img on the sd card with the one in the build folder.

## Using the APP
When all dependencies are installed and the app builded well, than after booting you should find the device ```dev/pruss0``` and the sd card should be mounted in ```/media```. I am currently working on a shell program to load pru code and execute it. When one is provided it is necessary to map the pru interupts to IRQ with:
```
sysctl dev.ti_pruss.0.irq.2.channel=2 # mapping channel 2 to irq2
sysctl dev.ti_pruss.0.irq.2.event=16 # mapping event 16 to irq2
sysctl dev.ti_pruss.0.irq.2.enable=1 # enable the irq2
sysctl dev.ti_pruss.0.global_interrupt_enable=1 # enable PRU global interrupts
```
This shoud create the device  ```/dev/pruss0.irqN```, where N is the interrupt channel and with the above example this should be 2. When runing test.bin from the pruexamples a interrupt should be triggered.
