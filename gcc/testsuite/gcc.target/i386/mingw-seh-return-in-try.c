/* { dg-do compile { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions" } */

int
return_from_seh (void)
{
  __try
    {
      return 1; /* { dg-error "return statement inside .__try. is not yet supported" } */
    }
  __except (1)
    {
      return 2;
    }
}
