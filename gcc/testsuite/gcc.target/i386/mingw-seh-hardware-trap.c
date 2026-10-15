/* { dg-do run { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions -O2" } */

#include <windows.h>

int
main (void)
{
  int value = 8;

  __try
    {
      value += 4;
      __asm__ volatile ("ud2");
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      return value == 12 ? 0 : 1;
    }

  return 2;
}
