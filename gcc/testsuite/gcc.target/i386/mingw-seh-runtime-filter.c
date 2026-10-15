/* { dg-do run { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions -O2 -fchecking=2" } */

#include <windows.h>
#include <excpt.h>
#include <stdint.h>

static int filter_calls;
static int recursive_catches;

static int
execute_without_arguments (void)
{
  ++filter_calls;
  return EXCEPTION_EXECUTE_HANDLER;
}

static int
execute_for_code (DWORD code)
{
  ++filter_calls;
  return code == 0x53454811 || code == 0x53454813
    || code == 0x53454814 || code == EXCEPTION_INT_DIVIDE_BY_ZERO
    ? EXCEPTION_EXECUTE_HANDLER
    : EXCEPTION_CONTINUE_SEARCH;
}

static void
recursive_seh (int depth)
{
  __try
    {
      if (depth == 0)
	RaiseException (0x53454814, 0, 0, 0);
      else
	recursive_seh (depth - 1);
    }
  __except (execute_for_code (GetExceptionCode ()))
    {
      ++recursive_catches;
    }
}

static int
execute_for_information (EXCEPTION_POINTERS *info)
{
  ++filter_calls;
  return info != 0 && info->ExceptionRecord != 0
    && ((info->ExceptionRecord->ExceptionCode == 0x53454812
         && info->ExceptionRecord->ExceptionFlags == 0
         && info->ExceptionRecord->NumberParameters == 2
         && info->ExceptionRecord->ExceptionInformation[0] == 0x12345678
         && info->ExceptionRecord->ExceptionInformation[1] == 0x87654321)
        || info->ExceptionRecord->ExceptionCode == 0x53454815
        || info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
    && info->ContextRecord != 0
    ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}

static int
continue_search (DWORD code)
{
  ++filter_calls;
  return code == 0x53454813 ? EXCEPTION_CONTINUE_SEARCH
    : EXCEPTION_EXECUTE_HANDLER;
}

static int
continue_after_ud2 (EXCEPTION_POINTERS *info)
{
  ++filter_calls;
  if (info == 0 || info->ExceptionRecord == 0 || info->ContextRecord == 0
      || info->ExceptionRecord->ExceptionCode != EXCEPTION_ILLEGAL_INSTRUCTION)
    return EXCEPTION_CONTINUE_SEARCH;

  info->ContextRecord->Rip += 2;
  return EXCEPTION_CONTINUE_EXECUTION;
}

int
main (void)
{
  int caught = 0;

  __try
    {
      RaiseException (0x53454810, 0, 0, 0);
    }
  __except (execute_without_arguments ())
    {
      ++caught;
    }

  __try
    {
      RaiseException (0x53454811, 0, 0, 0);
    }
  __except (execute_for_code (GetExceptionCode ()))
    {
      ++caught;
    }

  __try
    {
      /* Use the hardware instruction directly: signed division by zero is
         undefined in C, so a C expression would not be a stable fault test.  */
      __asm__ volatile ("xorl %%ecx, %%ecx\n\t"
                        "xorl %%edx, %%edx\n\t"
                        "movl $42, %%eax\n\t"
                        "idivl %%ecx"
                        ::: "rax", "rcx", "rdx", "cc", "memory");
    }
  __except (execute_for_code (GetExceptionCode ()))
    {
      ++caught;
    }

  recursive_seh (4);
  if (recursive_catches == 0)
    return 5;
  if (recursive_catches != 1)
    return 6;

  __try
    {
      ULONG_PTR parameters[2] = { 0x12345678, 0x87654321 };
      RaiseException (0x53454812, 0, 2, parameters);
    }
  __except (execute_for_information (GetExceptionInformation ()))
    {
      ++caught;
    }

  int *bad = (int *) (uintptr_t) 1;
  int value = 42;
  __try
    {
      __asm__ volatile ("movl (%0), %%eax"
                        : : "r" (bad) : "eax", "memory");
    }
  __except (execute_for_information (GetExceptionInformation ()))
    {
      if (value != 42)
	caught += 100;
      else
	++caught;
    }

  __try
    {
      __try
	{
	  RaiseException (0x53454813, 0, 0, 0);
	}
      __except (continue_search (GetExceptionCode ()))
	{
	  caught += 100;
	}
    }
  __except (execute_for_code (GetExceptionCode ()))
    {
      ++caught;
    }

  __try
    {
      __asm__ volatile ("ud2" ::: "memory");
      ++caught;
    }
  __except (continue_after_ud2 (GetExceptionInformation ()))
    {
      return 3;
    }

  /* A second information query verifies that the dispatcher supplies the
     current record for consecutive exceptions rather than stale state.  */
  __try
    {
      RaiseException (0x53454815, 0, 0, 0);
    }
  __except (execute_for_information (GetExceptionInformation ()))
    {
      ++caught;
    }

  return caught == 8 && filter_calls == 10 ? 0 : 1;
}
