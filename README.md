# rtems-pru
Application for developing/testing the pru-drivers on rtems.

## Dependencies
This application needs the mmap patch in rtems-libbsd, that is currently not mainline.
Also rtems-libbsd has to be build with the following argument.
```
--buildset=buildset/everything.ini
```
## Building all dependencies
1. A rtems installation is needed.
Please take a look atthe rtems Quickstart-Guide on this.
```
https://docs.rtems.org/branches/master/user/start/index.html
```

2. It is necessary to use my current rtems-libbsd Repository.
The LibBSD drivers are not committed yet.
So please install rtems-libbsd from here:
```
https://github.com/nilhoel1/rtems-libbsd/tree/ti_pruss
```

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
