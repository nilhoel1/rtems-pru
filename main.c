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

#include <rtems/rtems-fdt-shell.h>

#include <bsp/pruss-shell.h>

#include <bsp/prussdrv.h>
#include <bsp/pruss_intc_mapping.h>

#include <machine/rtems-bsd-config.h>

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

int shell_pruss_loader(int argc, char **argv) {
  if (argc != 2 && argc != 3) {
    printf("Usage: %s loader text.bin [data.bin]\n", argv[0]);
    return 1;
  }

  prussdrv_init();
  if (prussdrv_open(PRU_EVTOUT_0) == -1) {
    printf("prussdrv_open() failed\n");
    return 1;
  }

  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
  prussdrv_pruintc_init(&pruss_intc_initdata);

  printf("Executing program and waiting for termination\n");
  if (argc == 3) {
    if (prussdrv_load_datafile(0 /* PRU0 */, argv[2]) < 0) {
      fprintf(stderr, "Error loading %s\n", argv[2]);
      exit(-1);
    }
  }
  if (prussdrv_exec_program(0 /* PRU0 */, argv[1]) < 0) {
    fprintf(stderr, "Error loading %s\n", argv[1]);
    exit(-1);
  }

  // Wait for the PRU to let us know it's done
  prussdrv_pru_wait_event(PRU_EVTOUT_0);
  printf("All done\n");

  prussdrv_pru_disable(0 /* PRU0 */);
  prussdrv_exit();

  return 0;
}


int
main(int argc, char** argv)
{
  rtems_status_code sc = RTEMS_SUCCESSFUL;

  printf("\nRTEMS BBB PRU Tester\n\n");

  rtems_bsd_initialize();

  rtems_fdt_add_shell_command();

  rtems_shell_add_cmd ("pruss", "misc",
                       "Test PRU", rtems_pruss_shell_command);

  rtems_shell_add_cmd ("loader", "misc",
                       "Test PRU", shell_pruss_loader);

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
