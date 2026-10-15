/* { dg-do run { target x86_64-w64-mingw* } } */
/* { dg-options "-fms-extensions -O2 -flto" } */

#include <windows.h>
#include <excpt.h>

static __attribute__((noinline)) void
raise_seh ()
{
  RaiseException (0xe1234567, 0, 0, 0);
}

int
main ()
{
  volatile int result = 0;
  __try
    {
      raise_seh ();
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      result = 42;
    }
  return result != 42;
}
