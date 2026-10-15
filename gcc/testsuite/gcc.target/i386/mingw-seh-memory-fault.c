/* { dg-do run { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions -O2 -fchecking=2" } */

#include <windows.h>
#include <stdint.h>

int
main (void)
{
  int *bad = (int *) (uintptr_t) 1;
  int value = 42;

  __try
    {
      __asm__ volatile ("movl (%0), %%eax"
                        : : "r" (bad) : "eax", "memory");
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      if (value != 42)
	return 1;
      value = 17;
    }

  if (value != 17)
    return 2;

  value = 73;
  __try
    {
      __asm__ volatile ("movl %1, (%0)"
                        : : "r" (bad), "r" (value) : "memory");
      value = 99;
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      if (value != 73)
	return 3;
      value = 29;
    }

  return value == 29 ? 0 : 4;
}
