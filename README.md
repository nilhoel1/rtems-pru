# rtems-pru
Application for developing/testing the pru-drivers on rtems.

## Dependencies
This application needs the mmap patch in rtems-libbsd, that is currently not mainline.
Also rtems-libbsd has to be build with the following argument.
```
--buildset=buildset/everything.ini
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

Now the executables can be found in the build folder.
