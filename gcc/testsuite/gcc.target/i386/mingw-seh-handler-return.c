/* { dg-do run { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions -O2 -fchecking=2" } */

#include <windows.h>
#include <excpt.h>
#include <stdint.h>

static int
execute_with_information (EXCEPTION_POINTERS *info)
{
  return info != 0 && info->ExceptionRecord != 0 && info->ContextRecord != 0
    ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}

static int
continue_after_ud2 (EXCEPTION_POINTERS *info)
{
  if (info == 0 || info->ContextRecord == 0)
    return EXCEPTION_CONTINUE_SEARCH;

  info->ContextRecord->Rip += 2;
  return EXCEPTION_CONTINUE_EXECUTION;
}

static int
check_handler_return (void)
{
  int *bad = (int *) (uintptr_t) 1;
  int value = 42;

  __try
    {
      __asm__ volatile ("movl (%0), %%eax"
                        : : "r" (bad) : "eax", "memory");
    }
  __except (execute_with_information (GetExceptionInformation ()))
    {
      if (value != 42)
	return 4;
    }

  __try
    {
      __asm__ volatile ("ud2" ::: "memory");
    }
  __except (continue_after_ud2 (GetExceptionInformation ()))
    {
      return 3;
    }

  return 0;
}

int
main (void)
{
  return check_handler_return ();
}
