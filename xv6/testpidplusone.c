/**
 * (guanzhou) DIS W2 user program.
 */

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{

  // What `printf` are you calling here? Is it the C standard library?
  printf(1, "my pid = %d\n", getpid());
  printf(1, "my pid + 1 = %d\n", getpidplusone());

  exit();
}
