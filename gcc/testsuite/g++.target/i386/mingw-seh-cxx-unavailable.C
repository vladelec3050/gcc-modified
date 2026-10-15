/* { dg-do compile { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions" } */

struct resource
{
  ~resource () { }
};

void
seh_with_destructor ()
{
  resource r;
  /* { dg-error "__try.*not yet supported in C\+\+" } */
  __try
    {
      throw 1;
    }
  __except (1)
    {
    }
}
