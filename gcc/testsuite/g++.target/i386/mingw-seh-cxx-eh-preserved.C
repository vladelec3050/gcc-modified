/* { dg-do run { target *-*-mingw* } } */
/* { dg-options "-O2 -fchecking=2" } */

struct guard
{
  int *destructions;

  ~guard () { ++*destructions; }
};

static void
throw_from_nested_scope (int *destructions)
{
  guard local = { destructions };
  throw 17;
}

int
main ()
{
  int destructions = 0;
  try
    {
      guard outer = { &destructions };
      throw_from_nested_scope (&destructions);
    }
  catch (int value)
    {
      if (value != 17 || destructions != 2)
        return 1;
    }

  return destructions == 2 ? 0 : 2;
}
