#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(void)
{
    int t;

  printf(1, "Testing NumFreePages\n");
  t = getNumFreePages();
  printf(1, "%d\n", t);
  printf(1, "Hello, world!\n");
  printf(1, "Testing NumFreePages\n");
  t = getNumFreePages();
  printf(1, "%d\n", t);
  // int t;
  // printf(1,"Testing NumPhysPages\n");
  // t = getNumPhysPages();
  // printf(1, "%d\n", t);
  // printf(1,"Testing NumVirtPages\n");
  // t = getNumVirtPages();
  // printf(1, "%d\n", t);

  
  exit();
}
