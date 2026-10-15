/* { dg-do run { target i?86-*-mingw* } } */
/* { dg-options "-fms-extensions -O0" } */
#include <excpt.h>

int
main (void)
{
  volatile int result = 0;
  __try
    {
      /* Keep the intentional fault visible to optimizing compilers.  A C
	 null dereference is undefined behavior and may be replaced with a trap.  */
      __asm__ volatile ("xorl %%eax, %%eax\n\tmovl (%%eax), %%eax"
			: : : "eax", "memory");
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      result = 42;
    }
  return result != 42;
}
