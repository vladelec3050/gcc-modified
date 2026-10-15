/* { dg-do compile { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions" } */

#include <windows.h>
#include <excpt.h>

int
unsupported_arbitrary_filter (int local_state)
{
  __try
    {
      RaiseException (0x53454820, 0, 0, 0);
    }
  __except (GetExceptionCode () == 0x53454820 && local_state ? 1 : 0)
    /* { dg-error "initial MinGW x64 SEH implementation requires" } */
    {
      return 1;
    }

  return 0;
}
