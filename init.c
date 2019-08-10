/*
 *  Configuration information
 */

#include <stdlib.h>

#include <bsp.h>
#include <rtems/shell.h>

static const char* argv[2] = {
  "pru.exe",
  NULL
};

extern int main(int argc, const char** argv);

rtems_task Init(rtems_task_argument ignored) {
  exit(main(1, argv));
}

/*
 * Configure LibBSD.
 */
#define BUS_DEBUG
#define RTEMS_BSD_CONFIG_DOMAIN_PAGE_MBUFS_SIZE (64 * 1024 * 1024)
#define RTEMS_BSD_CONFIG_NET_PF_UNIX
#define RTEMS_BSD_CONFIG_NET_IP_MROUTE
#define RTEMS_BSD_CONFIG_NET_IP6_MROUTE
#define RTEMS_BSD_CONFIG_NET_IF_BRIDGE
#define RTEMS_BSD_CONFIG_NET_IF_LAGG
#define RTEMS_BSD_CONFIG_NET_IF_VLAN
#define RTEMS_BSD_CONFIG_BSP_CONFIG

#define RTEMS_BSD_CONFIG_INIT

#include <machine/rtems-bsd-config.h>

#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_UNLIMITED_OBJECTS         1
#define CONFIGURE_UNLIMITED_ALLOCATION_SIZE (20)

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_MEMORY_OVERHEAD           (2048)

#define CONFIGURE_MAXIMUM_POSIX_KEYS        (50)
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS   (2)

/*
 * Shell.
 */

#include <machine/rtems-bsd-commands.h>
#include <rtems/netcmds-config.h>

#include <bsp/irq-info.h>
//#include <bsp/nexus-devices.h>

#define CONFIGURE_SHELL_USER_COMMANDS \
  &bsp_interrupt_shell_command, \
  &rtems_shell_ARP_Command, \
  &rtems_shell_HOSTNAME_Command, \
  &rtems_shell_PING_Command, \
  &rtems_shell_ROUTE_Command, \
  &rtems_shell_NETSTAT_Command, \
  &rtems_shell_IFCONFIG_Command, \
  &rtems_shell_TCPDUMP_Command, \
  &rtems_shell_SYSCTL_Command, \
  &rtems_shell_VMSTAT_Command

#define CONFIGURE_SHELL_COMMAND_CPUINFO
#define CONFIGURE_SHELL_COMMAND_CPUUSE
#define CONFIGURE_SHELL_COMMAND_PERIODUSE
#define CONFIGURE_SHELL_COMMAND_STACKUSE
#define CONFIGURE_SHELL_COMMAND_PROFREPORT

#define CONFIGURE_SHELL_COMMAND_CP
#define CONFIGURE_SHELL_COMMAND_PWD
#define CONFIGURE_SHELL_COMMAND_LS
#define CONFIGURE_SHELL_COMMAND_LN
#define CONFIGURE_SHELL_COMMAND_LSOF
#define CONFIGURE_SHELL_COMMAND_CHDIR
#define CONFIGURE_SHELL_COMMAND_CD
#define CONFIGURE_SHELL_COMMAND_MKDIR
#define CONFIGURE_SHELL_COMMAND_RMDIR
#define CONFIGURE_SHELL_COMMAND_CAT
#define CONFIGURE_SHELL_COMMAND_MV
#define CONFIGURE_SHELL_COMMAND_RM
#define CONFIGURE_SHELL_COMMAND_MALLOC_INFO
#define CONFIGURE_SHELL_COMMAND_SHUTDOWN

#define CONFIGURE_SHELL_COMMANDS_INIT

#include <rtems/shellconfig.h>

/**
 * Configure drivers.
 */
#define CONFIGURE_MAXIMUM_DRIVERS                  10
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

/**
 * File systems
 */
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS  200
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_FILESYSTEM_IMFS
#define CONFIGURE_FILESYSTEM_NFS
#define CONFIGURE_FILESYSTEM_RFS

#define CONFIGURE_INIT

#include <rtems/confdefs.h>
