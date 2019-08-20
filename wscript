#
# Build the PRU system. Based on the example in rtems_waf/README.
#

from __future__ import print_function

import os

rtems_version = "5"

try:
    import rtems_waf.rtems as rtems
    import rtems_waf.rtems_bsd as rtems_bsd
    import rtems_waf.rootfs as rtems_rootfs
except:
    print('error: no rtems_waf git submodule; see README.waf', file = stderr)
    import sys
    sys.exit(1)

def init(ctx):
      rtems.init(ctx, version = rtems_version, long_commands = True)
      rtems_bsd.init(ctx)

def options(opt):
    rtems.options(opt)
    rtems_bsd.options(opt)

def bsp_configure(conf, arch_bsp):
    rtems_bsd.bsp_configure(conf, arch_bsp)
    conf.env['BOOT_IMAGE'] = conf.find_program('rtems-boot-image')

def configure(conf):
    rtems.configure(conf, bsp_configure)

def build(bld):
    rtems.build(bld)

    exe = 'pru.exe'
    cflags = ['-O1','-g', '-Wall']
    includes = ['include']

    bld.objects(name = 'libpru',
                features = 'c',
                source = ['libpru/pru.c',
                            'libpru/ti-pru.c'],
                includes = includes,
                cflags = cflags)

    bld.objects(name = 'pruss-shell',
                features = 'c',
                source = ['pruss-shell.c'],
                includes = includes,
                cflags = cflags)

    bld(features = 'c cprogram',
        target = exe,
        source = ['main.c',
                  'init.c',],
        includes = includes,
        cflags = cflags,
        use = ['libpru',
                'pruss-shell'],
        lib = ['debugger',
                'rtemscpu',
                'z'])

    bimg = ' '.join(bld.env.BOOT_IMAGE)
    bbsp = 'u-boot-beaglebone'
    bld(rule = bimg + ' -l bimg.txt -b ' + bbsp + ' --convert-kernel -o ${TGT} ${SRC}',
        name = 'boot-image',
        target = exe + '.img',
        source = exe,
        color = 'CYAN')
