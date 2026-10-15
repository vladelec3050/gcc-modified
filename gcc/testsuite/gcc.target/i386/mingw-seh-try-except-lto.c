/* { dg-do run { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions -O2 -flto" } */

/* Reuse the runtime scenarios from the non-LTO test.  */
#include "mingw-seh-try-except.c"
