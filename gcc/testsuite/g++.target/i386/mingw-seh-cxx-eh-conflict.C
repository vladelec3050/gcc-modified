/* { dg-do compile { target x86_64-w64-mingw* } } */
/* { dg-options "-fms-extensions -O2" } */

void
seh_and_cpp_eh_must_be_rejected ()
/* { dg-error "native Microsoft SEH cannot yet be combined with C\+\+ cleanup" "" { target *-*-* } . } */
{
  __try
    {
      volatile int value = 1;
      (void) value;
    }
  __except (1)
    {
    }
  try
    {
      throw 1;
    }
  catch (...)
    {
    }
}
