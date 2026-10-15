/* { dg-do compile { target x86_64-w64-mingw* } } */
/* { dg-options "-fms-extensions -O2" } */

struct guard
{
  ~guard ();
};

guard::~guard () { }

void
seh_raii_must_be_rejected ()
{
  __try
/* { dg-error "C\+\+ destructors and language exception handling inside" "" { target *-*-* } .-1 } */
    {
      guard g;
      volatile int value = 0;
      (void) value;
    }
  __except (1)
    {
    }
}
