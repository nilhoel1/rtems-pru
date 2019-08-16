/*
 * Main entry.
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <rtems/console.h>
#include <rtems/rtems-debugger.h>
#include <rtems/rtems-debugger-remote-tcp.h>
#include <rtems/shell.h>
#include <rtems/media.h>
#include <rtems/bdbuf.h>

#include <machine/rtems-bsd-config.h>

#include <libpru/libpru.h>

extern rtems_shell_cmd_t rtems_shell_DEBUGGER_Command;

#define EVT_MOUNTED		RTEMS_EVENT_9
#define STACK_SIZE_MEDIA_SERVER	(64 * 1024)
#define PRIO_MEDIA_SERVER	200

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

static rtems_id wait_mounted_task_id = RTEMS_INVALID_ID;

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

static rtems_status_code
media_listener(rtems_media_event event, rtems_media_state state,
    const char *src, const char *dest, void *arg)
{
	printf(
		"media listener: event = %s, state = %s, src = %s",
		rtems_media_event_description(event),
		rtems_media_state_description(state),
		src
	);

	if (dest != NULL) {
		printf(", dest = %s", dest);
	}

	if (arg != NULL) {
		printf(", arg = %p\n", arg);
	}

	printf("\n");

	if (event == RTEMS_MEDIA_EVENT_MOUNT &&
	    state == RTEMS_MEDIA_STATE_SUCCESS) {
		rtems_event_send(wait_mounted_task_id, EVT_MOUNTED);
	}

	return RTEMS_SUCCESSFUL;
}


void
libbsdhelper_init_sd_card(rtems_task_priority prio_mediaserver)
{
	rtems_status_code sc;

	wait_mounted_task_id = rtems_task_self();

	sc = rtems_bdbuf_init();
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_initialize();
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_listener_add(media_listener, NULL);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_server_initialize(
	    prio_mediaserver,
	    STACK_SIZE_MEDIA_SERVER,
	    RTEMS_DEFAULT_MODES,
	    RTEMS_DEFAULT_ATTRIBUTES
	    );
	assert(sc == RTEMS_SUCCESSFUL);
}

rtems_status_code
libbsdhelper_wait_for_sd(void)
{
	puts("waiting for SD...\n");
	rtems_status_code sc;
	const rtems_interval max_mount_time = 30000 /
	    rtems_configuration_get_milliseconds_per_tick();
	rtems_event_set out;

	sc = rtems_event_receive(EVT_MOUNTED, RTEMS_WAIT, max_mount_time, &out);
	assert(sc == RTEMS_SUCCESSFUL || sc == RTEMS_TIMEOUT);

	return sc;
}


int
usage(void)
{
	fprintf(stderr, "usage: %s -t type [-p pru-number] [-edrw] [program]\n",
	    "pructl");
	return 99;
}

int
pructl(int argc, char **argv)
{
	printk("===== pructl =====\n");
	int ch;
	int reset, enable, disable, wait;
	const char *type;
	unsigned int pru_number;
	pru_type_t pru_type;
	pru_t pru;
        int error;

	reset = enable = disable = pru_number = wait = 0;
	type = NULL;
	error = 0;
	while ((ch = getopt(argc, argv, "t:p:edrw")) != -1) {
		switch (ch) {
		case 't':
			type = optarg;
			break;
		case 'p':
			pru_number = (unsigned int)strtoul(optarg, NULL, 10);
			break;
		case 'e':
			enable = 1;
			break;
		case 'd':
			disable = 1;
			break;
		case 'r':
			reset = 1;
			break;
		case 'w':
			wait = 1;
			break;
		case '?':
		default:
			return usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (enable && disable) {
		fprintf(stderr, "%s: conflicting options: -e and -d\n",
		    "pructl");
		return usage();
	}
	if (type == NULL) {
		fprintf(stderr, "%s: missing type (-t)\n", "pructl");
		return usage();
	}
  printk("=== pru_name_to_type\n");
	pru_type = pru_name_to_type(type);
	if (pru_type == PRU_TYPE_UNKNOWN) {
		fprintf(stderr, "%s: invalid type '%s'\n", "pructl",
		    type);
		return 2;
	}
  printk("=== pru_alloc\n");
	pru = pru_alloc(pru_type);
	if (pru == NULL) {
		fprintf(stderr, "%s: unable to allocate PRU structure: %s\n",
		    "pructl", strerror(errno));
		return 3;
	}
	if (reset) {
    printk("=== pru_reset\n");
		error = pru_reset(pru, pru_number);
		if (error) {
			fprintf(stderr, "%s: unable to reset PRU %d\n",
			    "pructl", pru_number);
			return 4;
		}
	}
	if (argc > 0) {
    printk("=== pru_upload\n");
		error = pru_upload(pru, pru_number, argv[0]);
		if (error) {
			fprintf(stderr, "%s: unable to upload %s: %s\n",
			    "pructl", argv[0], strerror(errno));
			return 5;
		}
	}
	if (enable) {
    printk("=== pru_enable\n");
		error = pru_enable(pru, pru_number, 0);
		if (error) {
			fprintf(stderr, "%s: unable to enable PRU %d\n",
			    "pructl", pru_number);
			return 6;
		}
	}
	if (disable) {
    printk("=== pru_disable\n");
		error = pru_disable(pru, pru_number);
		if (error) {
			fprintf(stderr, "%s: unable to disable PRU %d\n",
			    "pructl", pru_number);
			return 7;
		}
	}
	if (wait) {
    printk("=== pru_wait\n");
		error = pru_wait(pru, pru_number);
		if (error) {
			fprintf(stderr, "%s: unable to wait for PRU %d\n",
			    "pructl", pru_number);
			return 8;
		}
	}
	printk("===== Free PRu =====\n");
	pru_free(pru);

	return 0;
}

int
main(int argc, char** argv)
{
  rtems_status_code sc;


  printf("\nRTEMS BBB PRU Tester\n\n");
  
  rtems_bsd_initialize();

  printf("\nLibBSD initialized\n\n");


  libbsdhelper_init_sd_card(PRIO_MEDIA_SERVER);

    rtems_shell_add_cmd ("pructl", "misc",
                       "Test PRU", pructl);

  /* Wait for the SD card */
	sc = libbsdhelper_wait_for_sd();
	if (sc == RTEMS_SUCCESSFUL) {
		printf("SD: OK\n");
	} else {
		printf("ERROR: SD could not be mounted after timeout\n");
	}

  sc = RTEMS_SUCCESSFUL;

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
