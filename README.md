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
Create an image with :
```
dd if=PATH_TO_SD-IMAGE_IN_FDT_FOLDER of=PATH_TO_SD_CARD bs=1M
```
Also tools like etcher can be used. 
The image with the device tree can be found in the fdt folder.
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
