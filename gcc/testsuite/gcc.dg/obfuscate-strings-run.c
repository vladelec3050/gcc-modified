/* { dg-do run } */
/* { dg-options "-O0 -fobfuscate-strings" } */

extern int puts (const char *);

static const char *saved;

static void
keep (const char *p)
{
  saved = p;
}

int
main (void)
{
  keep ("GCC_OBFUSCATION_RUNTIME_TEST_7f91");
  const char expected[] = {
    'G', 'C', 'C', '_', 'O', 'B', 'F', 'U', 'S', 'C', 'A', 'T', 'I', 'O',
    'N', '_', 'R', 'U', 'N', 'T', 'I', 'M', 'E', '_', 'T', 'E', 'S', 'T',
    '_', '7', 'f', '9', '1', 0
  };
  for (unsigned i = 0; i < sizeof expected; ++i)
    if (saved[i] != expected[i])
      return 1;
  puts (saved);
  return 0;
}
