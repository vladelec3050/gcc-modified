/* { dg-do compile { target i?86-*-mingw* } } */
/* { dg-options "-fms-extensions -O2" } */

struct guard
{
  ~guard () { }
};

void
seh_with_raii ()
{
  guard g;
  __try
    {
      throw 1;
    }
  __except (1)
    {
    }
}
/* { dg-error "C\+\+ cleanup, exception handling, and non-local control flow" "" { target *-*-* } .-1 } */
