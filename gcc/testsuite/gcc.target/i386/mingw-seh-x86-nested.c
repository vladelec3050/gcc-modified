/* { dg-do run { target i?86-*-mingw* } } */
/* { dg-options "-fms-extensions -O0" } */
#include <windows.h>
#include <excpt.h>

int
main (void)
{
  volatile int result = 0;
  __try
    {
      __try
	{
	  RaiseException (0xe1234567, 0, 0, 0);
	}
      __except (EXCEPTION_CONTINUE_SEARCH)
	{
	  result = 1;
	}
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      result = 42;
    }
  return result != 42;
}
