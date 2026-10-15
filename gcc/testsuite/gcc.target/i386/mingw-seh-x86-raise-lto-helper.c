/* { dg-do compile { target i?86-*-mingw* } } */
/* { dg-options "-fms-extensions -O2 -flto" } */
#include <windows.h>

__attribute__((noinline)) void
raise_seh_from_other_file (void)
{
  RaiseException (0xe1234567, 0, 0, 0);
}
