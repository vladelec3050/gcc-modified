#include <windows.h>

__attribute__ ((noinline)) void
seh_raise_exception (void)
{
  RaiseException (0x12345678, 0, 0, 0);
}
