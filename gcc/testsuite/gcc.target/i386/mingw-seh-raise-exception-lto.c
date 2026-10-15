/* { dg-do run { target x86_64-*-mingw* } } */
/* { dg-options "-fms-extensions -O2 -flto" } */
/* { dg-additional-sources "mingw-seh-raise-helper.c" } */

#include "mingw-seh-raise-exception.c"
