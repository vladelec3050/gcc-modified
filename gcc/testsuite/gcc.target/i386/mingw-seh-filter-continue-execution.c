/* { dg-do compile { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions" } */

int
seh_continue_execution_filter (void)
{
  __try
    {
      __asm__ volatile ("nop");
    }
  __except (-1)
    {
      return 1;
    }
  return 0;
}

/* The native filter thunk must preserve the signed CONTINUE_EXECUTION value.  */
/* { dg-final { scan-assembler "movl \\$-1, %eax" } } */
