/* { dg-do run { target x86_64-w64-mingw* } } */
/* { dg-options "-fms-extensions -O2" } */

#include <excpt.h>

int
main ()
{
  volatile int result = 0;
  __try
    {
      __asm__ volatile ("xorl %%eax, %%eax\n\tmovl (%%eax), %%eax"
			: : : "eax", "memory");
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      result = 42;
    }
  return result != 42;
}
