#include "types.h"
#include "user.h"

void
do_syscall_good(void)
{
  int ticks = uptime();
  (void) ticks;
}

void
do_syscall_fail(void)
{
  int ret = kill(-3);
  (void) ret;
}

int
main(int argc, char *argv[])
{
  int num_syscalls, num_syscalls_good, i, mypid;

  if (argc != 3) {
    printf(1, "syscalls: must invoke as `syscalls N g`\n");
    exit();
  }

  num_syscalls = atoi(argv[1]);
  num_syscalls_good = atoi(argv[2]);

  if (num_syscalls <= 0 || num_syscalls_good <= 0
      || num_syscalls_good > num_syscalls) {
    printf(1, "syscalls: must have N > 0 and 0 < g <= N\n");
    exit();
  }

  // Need to reserve for the must-have getpid() call.
  mypid = getpid();

  // WHY THE COUNTERS BOTH TURN 3 HERE???

  for (i = 0; i < num_syscalls_good - 1; ++i)
    do_syscall_good();

  for (i = num_syscalls_good - 1; i < num_syscalls - 1; ++i)
    do_syscall_fail();

  printf(1, "%d %d\n", getnumsyscalls(mypid), getnumsyscallsgood(mypid));

  exit();
}
