/*
 * Main entry.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>

#include <rtems/console.h>
#include <rtems/rtems-debugger.h>
#include <rtems/rtems-debugger-remote-tcp.h>
#include <rtems/shell.h>

#include <machine/rtems-bsd-config.h>
#include <libpru.h>

extern rtems_shell_cmd_t rtems_shell_DEBUGGER_Command;

/*
 * DHCP is currently crashing on BBB
 */
#define NET_DHCP 1

#if NET_DHCP
 #define NET_IP      NULL
 #define NET_NETMASK NULL
 #define NET_GATEWAY NULL
#else
 #define NET_IP      "192.168.2.2"
 #define NET_NETMASK "255.255.255.0"
 #define NET_GATEWAY "192.168.2.1"
#endif

static bool
net_config(const char* iface,
	   const char* hostname,
	   const char* ip,
	   const char* netmask,
	   const char* gateway)
{
  FILE* rc_conf;

  rc_conf = fopen("/etc/rc.conf", "w");
  if (rc_conf == NULL) {
    printf("error: cannot create /etc/rc.conf\n");
    return false;
  }

  fprintf(rc_conf, "#\n# LibBSD Configuration\n#\n\n");
  fprintf(rc_conf, "hostname=\"%s\"\n", hostname);

  if (ip == NULL) {
    fprintf(rc_conf, "ifconfig_%s=\"DHCP\"\n", iface);
  }
  else {
    fprintf(rc_conf, "ifconfig_%s=\"inet %s netmask %s\"\n", iface, ip, netmask);
  }

  if ((ip != NULL) && gateway != NULL) {
    fprintf(rc_conf, "defaultrouter=\"%s\"\n", gateway);
  }

  if (ip == NULL) {
    fprintf(rc_conf, "\ndhcpcd_priority=\"%d\"\n", 200);
    fprintf(rc_conf, "dhcpcd_options=\"--nobackground --timeout 10\"\n");
  }

  //fprintf(rc_conf, "\ntelnetd_enable=\"YES\"\n");

  fclose(rc_conf);

  return true;
}

static void
debugger_config(void)
{
  rtems_printer printer;
  int           r;

  rtems_print_printer_fprintf(&printer, stdout);

  rtems_shell_add_cmd_struct(&rtems_shell_DEBUGGER_Command);

  r = rtems_debugger_register_tcp_remote();
  if (r < 0) {
    printf("error: TCP remote register: %s\n", strerror(errno));
    return;
  }

  r = rtems_debugger_start("tcp", "1122", 3, 1, &printer);
  if (r < 0) {
    printf("error: debugger start: %s\n", strerror(errno));
    return;
  }
}

int
main(int argc, char** argv)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  printf("\nRTEMS BBB PRU Tester\n\n");
  
  rtems_bsd_initialize();

  printf("\nLibBSD initialized\n\n");

  if (net_config("cpsw0", "bbbpru", NET_IP, NET_NETMASK, NET_GATEWAY)) {
    int  timeout = 10;
    bool trace = true;
    int  r = rtems_bsd_run_etc_rc_conf(timeout, trace);
    if (r < 0)
      printf("error: net /etc/rc.conf failed: %s\n", strerror(errno));
    debugger_config();
  }

  while (sc == RTEMS_SUCCESSFUL) {
    printf("Starting a shell ...\n");
    sc = rtems_shell_init("SHLL",
			  32 * 1024,
			  1,
			  CONSOLE_DEVICE_NAME,
			  true,
			  true,
			  NULL
			  );


  }
}
