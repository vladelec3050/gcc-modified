/* { dg-do run { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions -O2" } */

#include <windows.h>

static __attribute__ ((noinline)) void
raise_code (DWORD code)
{
  RaiseException (code, 0, 0, 0);
}

int
main (void)
{
  int result = 0;

  /* CONTINUE_SEARCH must skip the inner handler and continue dispatching
     through the outer lexical scope.  */
  __try
    {
      __try
	{
	  raise_code (0x53454801);
	}
      __except (EXCEPTION_CONTINUE_SEARCH)
	{
	  result += 100;
	}
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      result += 1;
    }

  /* A later exception in the same function must still select its own scope.  */
  __try
    {
      raise_code (0x53454802);
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      result += 2;
    }

  return result == 3 ? 0 : 1;
}
