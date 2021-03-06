
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "py-eval.h"

static char* usage =
"usage: py-eval [code_files]* expr_file\n"
"use - to reset the interpreter\n"
"see the README\n";

static int verbosity = 0;
static void verbose(char* fmt, ...);
static void crash(char* fmt, ...);
static void do_python_code(char* code_file);
static void do_python_eval(char* expr_file);

static void options(int argc, char* argv[]);

int
main(int argc, char* argv[])
{
  // Set up
  options(argc, argv);
  if (argc == optind) crash(usage);
  python_init();

  // Execute files
  int cf; // current file
  for (cf = optind; cf < argc-1; cf++)
  {
    char* code_file = argv[cf];
    if (strcmp(code_file, "-") == 0)
    {
      verbose("reset");
      python_reset();
      continue;
    }
    do_python_code(code_file);
  }

  do_python_eval(argv[cf]);

  // Clean up
  python_finalize();
  exit(EXIT_SUCCESS);
}

static void
options(int argc, char* argv[])
{
  int opt;
  while ((opt = getopt(argc, argv, "v")) != -1)
  {
    switch (opt)
    {
      case 'v': verbosity = 1; break;
      default: crash("option processing");
    }
  }
}

static void
do_python_code(char* code_file)
{
  verbose("code: %s", code_file);

  // Read Python code file
  char* code = slurp(code_file);
  if (code == NULL) crash("failed to read: %s", code_file);
  chomp(code);

  // Execute Python code
  bool rc = python_code(code);
  if (!rc) crash("python code failed.");
  free(code);
}

static void
do_python_eval(char* expr_file)
{
  verbose("eval: %s", expr_file);

  // Read Python expr file
  if (strcmp(expr_file, "-") == 0)
    crash("expr file cannot be -");
  char* expr = slurp(expr_file);
  if (expr == NULL) crash("failed to read: %s", expr_file);
  chomp(expr);

  // Do Python eval
  char* result;
  bool rc = python_eval(expr, &result);
  if (!rc) crash("python expr failed.");
  free(expr);
  printf("%s\n", result);
}

static void
verbose(char* fmt, ...)
{
  if (verbosity == 0) return;

  printf("py-eval: ");
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");
  fflush(stdout);
}

static void
crash(char* fmt, ...)
{
  printf("py-eval: abort: ");
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");

  exit(EXIT_FAILURE);
}
