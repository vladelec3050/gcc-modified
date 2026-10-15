/* { dg-do run { target i?86-*-mingw* } } */
/* { dg-options "-fms-extensions -O2 -flto" } */
/* { dg-additional-sources "mingw-seh-x86-raise-lto-helper.c" } */
extern void raise_seh_from_other_file (void);

int
main (void)
{
  volatile int result = 0;
  __try
    {
      raise_seh_from_other_file ();
      result = 1;
    }
  __except (1)
    {
      result = 42;
    }
  return result != 42;
}
