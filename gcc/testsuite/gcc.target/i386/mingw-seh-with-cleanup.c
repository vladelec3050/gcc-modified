/* { dg-do compile { target x86_64-*-mingw* } } */
/* { dg-options "-fexceptions -fms-extensions" } */

static volatile int cleanup_calls;

static void
cleanup (int *value)
{
	cleanup_calls += *value + 1;
}

extern void may_throw (void);

int
seh_and_gcc_cleanup (void)
{
  int value __attribute__ ((cleanup (cleanup))) = 0;

  __try
    {
      may_throw ();
      value = 1;
    }
  __except (1)
    {
      value = 2;
    }

  return value;
}

/* There is only one PE exception-handler slot per runtime function.  */
/* { dg-error "native Microsoft SEH cannot yet be combined with GCC language exception cleanup" "" { target *-*-* } 0 } */
