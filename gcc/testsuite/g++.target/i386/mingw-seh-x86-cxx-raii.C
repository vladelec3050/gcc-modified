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
/* { dg-error "__try.*not yet supported in C\+\+" "" { target *-*-* } .-9 } */
