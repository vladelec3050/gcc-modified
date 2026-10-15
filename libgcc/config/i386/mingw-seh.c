/* Native 32-bit Windows SEH support for GCC.
   Copyright (C) 2026 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   GCC is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#if defined (__i386__) && !defined (__x86_64__)

#include <windows.h>
#include <stdint.h>

/* The first two words must match EXCEPTION_REGISTRATION_RECORD.  The
   remaining words are private to this implementation and are initialized
   by __mingw_seh_register.  */
struct mingw_seh_registration
{
  struct mingw_seh_registration *next;
  EXCEPTION_DISPOSITION (__cdecl *handler) (PEXCEPTION_RECORD, void *,
						     PCONTEXT, void *);
  int filter;
  void *handler_ip;
  uintptr_t saved_esp;
  uintptr_t saved_ebp;
};

static void *
mingw_seh_get_fs0 (void)
{
  void *value;
  __asm__ volatile ("movl %%fs:0, %0" : "=r" (value));
  return value;
}

static void
mingw_seh_set_fs0 (void *value)
{
  __asm__ volatile ("movl %0, %%fs:0" : : "r" (value) : "memory");
}

static EXCEPTION_DISPOSITION __cdecl
mingw_seh_handler (PEXCEPTION_RECORD exception_record, void *establisher_frame,
		   PCONTEXT context, void *dispatcher_context)
{
  struct mingw_seh_registration *registration = establisher_frame;
  (void) dispatcher_context;

  /* The Windows dispatcher calls this once while searching.  A selected
     handler restores the protected frame's stack state and resumes at the
     compiler-generated handler label.  Pop the whole chain through this
     record, as the native unwind would do for a handler in this frame.  */
  if (exception_record->ExceptionFlags & EXCEPTION_UNWINDING)
    return ExceptionContinueSearch;

  if (registration->filter == EXCEPTION_CONTINUE_EXECUTION)
    return ExceptionContinueExecution;

  if (registration->filter != EXCEPTION_EXECUTE_HANDLER)
    return ExceptionContinueSearch;

  mingw_seh_set_fs0 (registration->next);
  context->Esp = registration->saved_esp;
  context->Ebp = registration->saved_ebp;
  context->Eip = (DWORD) (uintptr_t) registration->handler_ip;
  return ExceptionContinueExecution;
}

void
__mingw_seh_register (void *frame, int filter, void *handler_ip)
{
  struct mingw_seh_registration *registration = frame;
  registration->next = mingw_seh_get_fs0 ();
  registration->handler = mingw_seh_handler;
  registration->filter = filter;
  registration->handler_ip = handler_ip;
  mingw_seh_set_fs0 (registration);
}

void
__mingw_seh_unregister (void *frame)
{
  struct mingw_seh_registration *registration = frame;
  if (mingw_seh_get_fs0 () == registration)
    mingw_seh_set_fs0 (registration->next);
}

#endif /* __i386__ && !__x86_64__ */
