
__attribute__((noinline, noclone)) int
f1 (long long d)
{
  long long w = d / 0x1234567800000LL;
  return w;
}

int
main ()
{
  return f1 (4LL) - f1 (4LL);
}
