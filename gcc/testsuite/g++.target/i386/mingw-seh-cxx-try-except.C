/* { dg-do run { target x86_64-w64-mingw* } } */
/* { dg-options "-fms-extensions -O0" } */

#include <windows.h>
#include <excpt.h>

static int
check_code (EXCEPTION_POINTERS *info)
{
  return info->ExceptionRecord->ExceptionCode == 0xe1234567
    ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}

static int
continue_for_code (unsigned int code)
{
  return code == 0xe1234567 ? EXCEPTION_CONTINUE_SEARCH
    : EXCEPTION_EXECUTE_HANDLER;
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
	  __try
	    {
	      RaiseException (0xe1234567, 0, 0, 0);
	    }
	  __except (continue_for_code (GetExceptionCode ()))
	    {
	      result = 2;
	    }
	}
      __except (EXCEPTION_CONTINUE_SEARCH)
	{
	  result = 1;
	}
    }
  __except (check_code (GetExceptionInformation ()))
    {
      result = local + 25;
    }
  return result != 42;
}
