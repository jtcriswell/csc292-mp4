#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Address to modify within the code segment */
static const unsigned long address = 0x400808;

int
main (int argc, char ** argv) {
  /* Process ID of the victim */
  pid_t victim;

  /*
   * Determint the PID of the victim process.
   */
  if (argc < 2) {
    fprintf (stderr, "Invalid number of arguments\n");
    return -1;
  }
  victim = atoi (argv[1]);

  /*
   * Attach to the victim process and wait for it to stop.
   */
  ptrace (PTRACE_ATTACH, victim, 0, 0);
  waitpid (victim, 0, 0);

  /*
   * Modify the argument of the call in the code segment.
   */
  printf ("%lx\n", ptrace (PTRACE_PEEKTEXT, victim, address, 0));
  ptrace (PTRACE_POKETEXT, victim, address, 0x75c085000003b3e8);
  printf ("%lx\n", ptrace (PTRACE_PEEKTEXT, victim, address, 0));

  /*
   * Make the victim resume execution.
   */
  ptrace (PTRACE_CONT, victim, 0, 0);
  return 0;
}
