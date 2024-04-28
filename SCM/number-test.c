#include <stdio.h>
#include "scm.h"

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef FLOATS
# ifdef BIGDIG
unsigned long
big2ulong(SCM b)
{
  double ans = big2dbl(b);
#   ifndef RECKLESS
#    ifdef HAVE_LIMITS_H
  if (ans > ULONG_MAX)
    ASSERT(0, b, OVFLOW, "big2ulong");
#   endif
#  endif
  return (unsigned long) ans;
}

unsigned int
big2uint(SCM b)
{
  double ans = big2dbl(b);
#   ifndef RECKLESS
#    ifdef HAVE_LIMITS_H
  if (ans > UINT_MAX)
    ASSERT(0, b, OVFLOW, "big2uint");
#   endif
#  endif
  return (unsigned int) ans;
}

int
big2int(SCM b)
{
  double ans = big2dbl(b);
#   ifndef RECKLESS
#    ifdef HAVE_LIMITS_H
  if (ans > 0)
    {
      if (ans > INT_MAX)
	ASSERT(0, b, OVFLOW, "big2int");
    }
  else 
    {
      if (ans < INT_MIN)
	ASSERT(0, b, OVFLOW, "big2int");
    }
#   endif
#  endif
  return (int) ans;
}
# endif
#endif  

SCM
SCM_number_test(SCM arglist)
{
  int j = 1;
  SCM lptr;

  lptr = arglist;

  printf("Compiler flags: ");
#ifdef FLOATS
  printf("FLOATS ");
#endif
#ifdef BIGDIG
  printf("BIGDIG ");
#endif
#ifdef SINGLES
  printf("SINGLES ");
#endif
#ifdef BIGSMOBS
  printf("BIGSMOBS ");
#endif

  printf("\n");

  while (lptr != EOL)
    {
      SCM el = CAR(lptr);

      printf("Argument %d: ", j);
      if (C_COMPATIBLE_NUMBERP(el))
	printf("C_COMPATIBLE_NUMBER ");
      if (INTP(el))
	printf("INT ");
      if (FLOATP(el))
	printf("FLOAT ");
      if (INUMP(el))
	printf("INUM ");
      if (NIMP(el) && INEXP(el))
	printf("INEX ");
      if (NIMP(el) && REALP(el))
	printf("REAL ");
      if (NIMP(el) && CPLXP(el))
	printf("CPLX ");
      if (NIMP(el) && BIGP(el))
	printf("BIG ");
      
      printf("\n");
      
      if (NIMP(el))
	printf("CAR(el) was %0.8x\n", CAR(el));

      lptr = CDR(lptr);
      j++;
    }
  return UNSPECIFIED;
}

SCM
SCM_ulong_cvt(SCM arglist)
{
  int i = ilength(arglist);
  switch(i)
    {
    case 1:
      {
	SCM arg1 = CAR(arglist);
	unsigned long result = big2ulong(arg1);
	printf("Result was %u.\n", result);
	return UNSPECIFIED;
      }
    default:
      ASSERT(0, arglist, WNA, "SCM_ulong_cvt");
    }
}

SCM
SCM_uint_cvt(SCM arglist)
{
  int i = ilength(arglist);
  switch(i)
    {
    case 1:
      {
	SCM arg1 = CAR(arglist);
	unsigned int result = big2uint(arg1);
	printf("Result was %u.\n", result);
	return UNSPECIFIED;
      }
    default:
      ASSERT(0, arglist, WNA, "SCM_uint_cvt");
    }
}
      
SCM
SCM_int_cvt(SCM arglist)
{
  int i = ilength(arglist);
  switch(i)
    {
    case 1:
      {
	SCM arg1 = CAR(arglist);
	int result = big2int(arg1);
	printf("Result was %d.\n", result);
	return UNSPECIFIED;
      }
    default:
      ASSERT(0, arglist, WNA, "SCM_int_cvt");
    }
}


static iproc list_subrs[] = {
  {"number-test", SCM_number_test},
  {"ulong-cvt", SCM_ulong_cvt},
  {"uint-cvt", SCM_uint_cvt},
  {"int-cvt", SCM_int_cvt},
  {0, 0}};

void
init_SCM_number_test()
{
  init_iprocs(list_subrs, tc7_lsubr);
}
