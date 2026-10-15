/* { dg-do run { target i?86-*-mingw* } } */
/* { dg-options "-fms-extensions -O0" } */

#include <windows.h>
#include <excpt.h>

extern "C" __attribute__((noinline)) void
raise_from_callee ()
{
  RaiseException (0xe1234567, 0, 0, 0);
}

static volatile int initial_local = 17;

int
main ()
{
  int local = initial_local;
  volatile int result = 0;
  __try
    {
      __try
	{
	  raise_from_callee ();
	}
      __except (EXCEPTION_CONTINUE_SEARCH)
	{
	  result = 1;
	}
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      result = local + 25;
    }
  return result != 42;
}
