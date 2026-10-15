/* { dg-do run { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions -O2" } */

#include <windows.h>

static __attribute__ ((noinline)) void
raise_code (DWORD code)
{
  RaiseException (code, 0, 0, 0);
}

static __attribute__ ((noinline)) void
raise_deep (DWORD code)
{
  raise_code (code);
}

int
main (void)
{
  int caught = 0;

  /* A call that is the final statement in a protected region must be caught.  */
  __try
    {
      raise_deep (0x1001);
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      caught += 1;
    }

  /* The call after the inner scope belongs to the outer scope only.  */
  __try
    {
      __try
	{
	  raise_deep (0x1002);
	}
      __except (EXCEPTION_EXECUTE_HANDLER)
	{
	  caught += 2;
	}
      raise_deep (0x1003);
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      caught += 3;
    }

  /* The first machine-level fault after an empty inner scope belongs outside
     that scope, and is still handled by the enclosing protected region.  */
  __try
    {
      __try
	{
	}
      __except (EXCEPTION_EXECUTE_HANDLER)
	{
	  caught += 1000;
	}
      *(volatile int *) 1 = 1;
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      caught += 4;
    }

  /* Adjacent scopes must continue to select their own handlers.  */
  __try
    {
      raise_code (0x1005);
    }
      __except (EXCEPTION_EXECUTE_HANDLER)
    {
	caught += 5;
    }
  __try
    {
      raise_code (0x1006);
    }
      __except (EXCEPTION_EXECUTE_HANDLER)
    {
	caught += 6;
    }

  /* Repeated entry into one lexical scope must keep selecting its handler.  */
  for (int i = 0; i < 8; ++i)
    __try
      {
	if ((i & 1) == 0)
	  raise_deep (0x1100 + i);
	else
	  caught += 10;
      }
    __except (EXCEPTION_EXECUTE_HANDLER)
      {
	caught += 20;
      }

  return caught == 1 + 2 + 3 + 4 + 5 + 6 + 40 + 80 ? 0 : 1;
}
