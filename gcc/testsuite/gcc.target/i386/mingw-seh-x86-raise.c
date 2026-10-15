/* { dg-do run { target i?86-*-mingw* } } */
/* { dg-options "-fms-extensions -O0" } */
#include <windows.h>
#include <excpt.h>

static __attribute__((noinline)) void
raise_from_callee (void)
{
  RaiseException (0xe1234567, 0, 0, 0);
}

int
main (void)
{
  volatile int result = 0;
  __try
    {
      raise_from_callee ();
      result = 1;
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      result = 42;
    }
  return result != 42;
}
