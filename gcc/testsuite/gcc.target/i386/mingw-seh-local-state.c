/* { dg-do run { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions -O2" } */

#include <windows.h>

static __attribute__ ((noinline)) void
raise_code (void)
{
  RaiseException (0x53454803, 0, 0, 0);
}

int
main (int argc, char **argv)
{
  int value;
  int copy;

  if (argc > 1)
    value = 80;
  else
    value = 10;
  value += 7;
  copy = value * 2;

  __try
    {
      raise_code ();
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      value += 3;
      copy += value;
    }

  return value == 20 && copy == 54 ? 0 : 1;
}
