/* { dg-do run { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions -O2" } */
/* { dg-additional-sources "mingw-seh-raise-helper.c" } */

#include <windows.h>

extern void seh_raise_exception (void);

int
main (void)
{
  int result = 0;

  __try
    {
      seh_raise_exception ();
    }
  __except (EXCEPTION_EXECUTE_HANDLER)
    {
      result = 42;
    }

  return result != 42;
}
