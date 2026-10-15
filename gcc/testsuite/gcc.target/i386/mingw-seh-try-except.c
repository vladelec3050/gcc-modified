/* { dg-do run { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions -O2" } */

#include <windows.h>

static __attribute__ ((noinline)) int
access_violation (void)
{
  /* Avoid GCC's null-dereference optimization: Windows reserves the low
     address range, but a non-null volatile access still faults at runtime.  */
  volatile int *p = (volatile int *) 1;
  int result = 0;

  __try
    {
      *p = 1;
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      result = 42;
    }

  return result;
}

int
main (void)
{
  return access_violation () != 42;
}
