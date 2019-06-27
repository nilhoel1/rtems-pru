#include <stdio.h>
#include <stdlib.h>

#include <bsp/prussdrv.h>
#include <bsp/pruss_intc_mapping.h>
#include <bsp/pruss-shell.h>

#include <rtems/printer.h>

/**
 * The type of the shell handlers we have.
 */
typedef int (*rtems_pruss_shell_handler) (const rtems_printer* printer, int argc, char *argv[]);

/**
 * Table of handlers we parse to invoke the command.
 */
typedef struct {
  const char*                 name;    /**< The sub-command's name. */
  rtems_pruss_shell_handler   handler; /**< The sub-command's handler. */
  const char*                 help;    /**< The sub-command's help. */
}rtems_pruss_shell_cmd;
/**
 * Object print data.
 */
typedef struct
{
  const rtems_printer* printer;           /**< The RTEMS printer. */
  bool prussdrv_init;                     /**< Print if prssdrv is initialised. */
  int pruss_number;                       /**< The PRU number. */
  bool pruss0_open;                       /**< Print PRU0 is open. */
  bool pruss1_open;                       /**< Print PRU0 is open .*/
  unsigned int prruss0_host_interrupt;    /*< Print PRU0 host interrupt. */
  unsigned int prruss1_host_interrupt;    /*< Print PRU1 host interrupt .*/
  bool pruss0_executing;                  /**< Print PRU0 is running. */
  bool pruss1_executing;                  /**< Print PRU1 is running. */
  char* pruss0_file_name;                 /**< Print file running on pruss0. */
  char* pruss1_file_name;                 /**< Print file running on pruss01. */
} rtems_rtl_obj_print;

int
rtems_pruss_shell_status (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
}

/* Tries to Initialise PRU0 and returns -1 if failed. */
int
rtems_pruss_shell_init (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
  prussdrv_init();
  if (prussdrv_open(PRU_EVTOUT_0) == -1) {
    rtems_printf (printer, "prussdrv_open() failed\n");
    return 1;
  }

  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
  prussdrv_pruintc_init(&pruss_intc_initdata);

  return 0;
}

/* Loads binary file to the pruss.
Pruss has to be initialised first. */
int
rtems_pruss_shell_load (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
//  printf("Executing program and waiting for termination\n");
  if (argc == 2) {
    if (prussdrv_load_datafile(0 /* PRU0 */, argv[1]) < 0) {
      rtems_printf (printer, "Error loading %s\n", argv[1]);
      exit(-1);
    }
  }
  rtems_printf (&printer, "error: unknown option: %s\n", argv[argc]);
  return -1;
}

int
rtems_pruss_shell_run (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{

}

int
rtems_pruss_shell_stop (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
}

/**
 * Parse an argument.
 */
static bool
rtems_pruss_parse_opt (const char opt, int argc, char *argv[])
{
  size_t arg;
  for (arg = 1; arg < argc; ++arg)
  {
    if (argv[arg][0] == '-')
    {
      size_t len = strlen (argv[arg]);
      size_t i;
      for (i = 1; i < len; ++i)
        if (argv[arg][i] == opt)
          return true;
    }
  }
  return false;
}

static bool
rtems_pruss_check_opts (const rtems_printer* printer,
                      const char*          opts,
                      int                  argc,
                      char*                argv[])
{
  size_t olen = strlen (opts);
  size_t arg;
  for (arg = 1; arg < argc; ++arg)
  {
    if (argv[arg][0] == '-')
    {
      size_t len = strlen (argv[arg]);
      size_t i;
      for (i = 1; i < len; ++i)
      {
        bool found = false;
        size_t       o;
        for (o = 0; o < olen; ++o)
        {
          if (argv[arg][i] == opts[o])
          {
            found = true;
            break;
          }
        }
        if (!found)
        {
          rtems_printf (printer, "error: invalid option: %c (%s)\n",
                        argv[arg][i], argv[arg]);
          return false;
        }
      }
    }
  }
  return true;
}

static ssize_t
rtems_pruss_parse_arg_index (const char  opt,
                           const char* skip_opts,
                           int         argc,
                           char*       argv[])
{
  ssize_t arg;
  for (arg = 1; arg < argc; ++arg)
  {
    if (argv[arg][0] == '-')
    {
      /*
       * We can check the next char because there has to be a valid char or a
       * nul.
       */
      if (argv[arg][1] != '\0')
      {
        size_t len = skip_opts != NULL ? strlen (skip_opts) : 0;
        size_t i;
        for (i = 0; i < len; ++i)
        {
          if (skip_opts[i] == argv[arg][1])
          {
            ++arg;
            break;
          }
        }
      }
    }
    else
    {
      if (opt == ' ')
        return arg;
    }
    /*
     * Is this an option and does it match what we are looking for?
     */
    if (argv[arg][0] == '-' && argv[arg][1] == opt && arg < argc)
      return arg + 1;
  }
  return -1;
}

static const char*
rtems_pruss_parse_arg (const char  opt,
                     const char* skip_opts,
                     int         argc,
                     char*       argv[])
{
  ssize_t arg = rtems_rtl_parse_arg_index (opt, skip_opts, argc, argv);
  if (arg < 0)
    return NULL;
  return argv[arg];
}

static void
rtems_pruss_shell_usage (const rtems_printer* printer, const char* arg)
{
  rtems_printf (printer, "%s: PRU Loader\n", arg);
  rtems_printf (printer, "  %s [-hl] <command>\n", arg);
  rtems_printf (printer, "   where:\n");
  rtems_printf (printer, "     command: A Action for the PRU. See -l for a list plus help.\n");
  rtems_printf (printer, "     -h:      This help\n");
  rtems_printf (printer, "     -l:      The command list.\n");
}

int
rtems_pruss_shell_command (int argc, char* argv[])
{
  const rtems_pruss_shell_cmd table[] =
  {
    { "status", rtems_pruss_shell_status,
      "Display the status of the PRU" },
    { "init", rtems_pruss_shell_init,
      "\tInitialize/Open the PRU" },
    { "load", rtems_pruss_shell_load,
      "\tLoad code in the PRU" },
    { "run", rtems_pruss_shell_run,
      "\tActivate the PRU" },
    { "stop", rtems_pruss_shell_stop,
      "\tStop the PRU" }
  };

  rtems_printer printer;
  int           arg;
  int           t;

  rtems_print_printer_printf (&printer);

  for (arg = 1; arg < argc; arg++)
  {
    if (argv[arg][0] != '-')
      break;

    switch (argv[arg][1])
    {
      case 'h':
        rtems_pruss_shell_usage (&printer, argv[0]);
        return 0;
      case 'l':
        rtems_printf (&printer, "%s: commands are:\n", argv[0]);
        for (t = 0;
             t < (sizeof (table) / sizeof (const rtems_pruss_shell_cmd));
             ++t)
          rtems_printf (&printer, "  %s\t%s\n", table[t].name, table[t].help);
        return 0;
      default:
        rtems_printf (&printer, "error: unknown option: %s\n", argv[arg]);
        return 1;
    }
  }

  if ((argc - arg) < 1)
    rtems_printf (&printer, "error: you need to provide a command, try %s -h\n",
                  argv[0]);
  else
  {
    for (t = 0;
         t < (sizeof (table) / sizeof (const rtems_pruss_shell_cmd));
         ++t)
    {
      if (strncmp (argv[arg], table[t].name, strlen (argv[arg])) == 0)
        return table[t].handler (&printer, argc - 1, argv + 1);
    }
    rtems_printf (&printer, "error: command not found: %s (try -h)\n", argv[arg]);
  }

  return 1;
}
