/* { dg-do compile { target x86_64-w64-mingw* } } */
/* { dg-options "-fms-extensions -O2" } */

#include <excpt.h>

struct guard
{
  ~guard ();
};

guard::~guard () { }

void
seh_with_outer_raii_must_be_rejected ()
/* { dg-error "native Microsoft SEH cannot yet be combined with C\+\+ cleanup" "" { target *-*-* } . } */
{
  guard g;
  __try
    {
      volatile int value = 0;
      (void) value;
    }
  __except (EXCEPTION_CONTINUE_SEARCH)
    {
    }
}
