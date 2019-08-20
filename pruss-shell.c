#include <stdio.h>
#include <stdlib.h>

#include <libpru/libpru.h>
#include <pruss-shell.h>

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
 * Object data.
 */
pru_t pru;
/*
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
}*/

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

/*
static const char*
rtems_pruss_parse_arg (const char  opt,
                     const char* skip_opts,
                     int         argc,
                     char*       argv[])
{
  ssize_t arg = rtems_pruss_parse_arg_index (opt, skip_opts, argc, argv);
  if (arg < 0)
    return NULL;
  return argv[arg];
}*/

/**
 * Parse an argument.
 *//*
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
}*/

/** 
 * Prints the current PRU status
 */
int
rtems_pruss_shell_status (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
  return 0;
}

int
rtems_pruss_parse_pru_number (const rtems_printer* printer,
                         int                 argc,
                         char*               argv[])
{
  int pru_number_index;

  pru_number_index = rtems_pruss_parse_arg_index('0', NULL, argc, argv);
  if (pru_number_index > 0) {
    return 0;
  }

  pru_number_index = rtems_pruss_parse_arg_index('1', NULL, argc, argv);
  if (pru_number_index > 0) {
    return 1;
  }

  rtems_printf (printer, "error: No valid PRU number provided.\n");
  return -1;
}

/** 
 * Initialises the PRU.
 * This function calls PRU_alloc and pru_reset.
 * Expects pru-number -[0, 1] as argument.
 */
int
rtems_pruss_shell_init (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
  int ret;

  ret = rtems_pruss_shell_alloc (printer, argc, argv);
  if( ret != 0)
    return ret;
  
  ret = rtems_pruss_shell_reset (printer, argc, argv);
  return ret;
}

/** 
 * Allocates the PRU.
 * This function calls pru_alloc. 
 */
int
rtems_pruss_shell_alloc (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
  pru = pru_alloc(pru_name_to_type("ti"));
  if (pru == NULL) {
    rtems_printf (printer, "error: PRU allocation failed\n");
    return -1;
  }
  return 0;
}

/**
 * Resets the PRU.
 * This function calls PRU_reset. 
 * Expects pru-number -[0, 1] as argument.
 */
int
rtems_pruss_shell_reset (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
  int error;
  int pru_number;

  pru_number = rtems_pruss_parse_pru_number(printer , argc, argv);
  if (pru_number < 0){
    return -1;
  }

  error = pru_reset(pru, pru_number);
  if(error){
    rtems_printf (printer, "error: PRU reset failed\n");
    return -1;
  }

  return 0;
}

/** 
 * Loads binary file to the pruss.
 * Pruss has to be initialised first.
 * Expects pru-number -[0, 1] as argument.
 */
int
rtems_pruss_shell_upload (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
  if (pru == NULL) {
    rtems_printf (printer, "error: PRU not allocated\n");
  }
  int error;
  int pru_number;

  pru_number = rtems_pruss_parse_pru_number(printer , argc, argv);
  if (pru_number < 0){
    return -1;
  }

  error = pru_upload (pru, pru_number, argv[argc-1]);
  if (error){
    rtems_printf (printer, "error: PRU upload of %s failed\n", argv[argc-1]);
    return -1;
  }
  return 0;
}

/**
 * Starts the PRU.
 * PRU has to be initialised first. 
 * Expects pru-number -[0, 1] as argument.
 */
int
rtems_pruss_shell_start (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
  if (pru == NULL) {
    rtems_printf (printer, "error: PRU not allocated\n");
  }
  int error;
  int pru_number;

  pru_number = rtems_pruss_parse_pru_number(printer , argc, argv);
  if (pru_number < 0){
    return -1;
  }

  error = pru_enable (pru, pru_number, 0);
  if (error){
    rtems_printf (printer, "error: Starting PRU failed\n");
    return -1;
  }
  return 0;
}

/**
 * Steps one instruction on the PRU.
 * PRU has to be initialised first. 
 * Expects pru-number -[0, 1] as argument.
 */
int
rtems_pruss_shell_step (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
  if (pru == NULL) {
    rtems_printf (printer, "error: PRU not allocated\n");
  }
  int error;
  int pru_number;

  pru_number = rtems_pruss_parse_pru_number(printer , argc, argv);
  if (pru_number < 0){
    return -1;
  }

  error = pru_enable (pru, pru_number, 1);
  if (error){
    rtems_printf (printer, "error: PRU step failed\n");
    return -1;
  }
  return 0;
}

/**
 * Stops the PRU.
 * PRU has to be initialized first.
 * Expects pru-number -[0, 1] as argument.
 */
int
rtems_pruss_shell_stop (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
  if (pru == NULL) {
    rtems_printf (printer, "error: PRU not allocated\n");
  }
  int error;
  int pru_number;

  pru_number = rtems_pruss_parse_pru_number(printer , argc, argv);
  if (pru_number < 0){
    return -1;
  }

  error = pru_disable (pru, pru_number);
  if (error){
    rtems_printf (printer, "error: PRU stop failed\n");
    return -1;
  }
  return 0;
}

/**
 * Starts PRU and waits for the PRU to return.
 * PRU has to be initialized first.
 * Expects pru-number [0, 1] as argument.
 */
int
rtems_pruss_shell_wait (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
  if (pru == NULL) {
    rtems_printf (printer, "error: PRU not allocated\n");
  }
  int error;
  int pru_number;

  pru_number = rtems_pruss_parse_pru_number(printer , argc, argv);
  if (pru_number < 0){
    return -1;
  }

  error = pru_enable (pru, pru_number, 0);
  if (error){
    rtems_printf (printer, "error: PRU start failed\n");
    return -1;
  }
  error = pru_wait (pru, pru_number);
  if (error) {
    rtems_printf (printer, "error: PRU wait failed\n");
    return -1;
  }
  return 0;
}

/**
 * Frees the PRU obejct memory.
 * PRU has to be initialized first.
 */
int
rtems_pruss_shell_free (const rtems_printer* printer,
                        int                  argc,
                        char*                argv[])
{
  if (pru == NULL) {
    rtems_printf (printer, "error: PRU is already free\n");
  }
  pru_free(pru);
  return 0;
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
    { "init", rtems_pruss_shell_init,
      "\tAllocates and resets PRU" },
    { "alloc", rtems_pruss_shell_alloc,
      "\tAllocated the PRU memory" },
    { "reset", rtems_pruss_shell_reset,
      "\tResets the PRU, reset -[01]" },
    { "upload", rtems_pruss_shell_upload,
      "\tUploads file to PRU, upload -[01] <file>" },
    { "start", rtems_pruss_shell_start,
      "\tStarts the PRU, start -[01]" },
    { "step", rtems_pruss_shell_step,
      "\tExecutes singe command on PRU, step -[01]" },
    { "stop", rtems_pruss_shell_stop,
      "\tStops the PRU, stop -[01]" },
    { "wait", rtems_pruss_shell_wait,
      "\tStarts the PRU and waits for completion, wait -[01]" },
    { "free", rtems_pruss_shell_free,
      "\tFrees the PRU Object memory" }
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
