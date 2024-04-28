#include <string.h>
#include "SCMGlobalDispatch.h"

ClassController *classController;
HashTable hashTable;

static iproc list_subrs[] = {
  {"->", SCM_Global_Dispatch},
  {"c++-object?", SCM_CPlusPlus_ObjectP},
  {0, 0}};

SCM
SCM_Global_Dispatch(SCM arglist, ...)
{
  int i;
  char *method_name;
  SCM object;
  long object_type;
  SCM method;
  SCM method_arglist;
  ClassNode *node;
  SCM retval;

  i = ilength(arglist);
  ASSERT((i >= 2), arglist, WNA, "SCM_Global_Dispatch");
  object = CAR(arglist);
  ASSERT(CELLP(object), object, ARG1, "SCM_Global_Dispatch");
  ASSERT(((CAR(object) & 127) == tc7_smob), object, ARG1, "SCM_Global_Dispatch");
  object_type = TYP32(object);
  method = CAR(CDR(arglist));
  ASSERT(CELLP(method) && SYMBOLP(method), method, ARG2, "SCM_Global_Dispatch");
  method_arglist = cons(object, CDR(CDR(arglist)));
  method_name = CHARS(method);
  node = classController->binarySearchByType(object_type);
  if (!node)
    {
      ASSERT(0, object, ARG1, "SCM_Global_Dispatch");
    }
  else
    {
      retval = (*(node->dispatch_func))(method_name, method_arglist);
      if (retval)
	return retval;
      else
	{
	  ASSERT(0, object, ARG1, "SCM_Global_Dispatch");
	}
    }
}

SCM
SCM_CPlusPlus_ObjectP(SCM arglist, ...)
{
  int i;
  SCM object;
  long object_type;
  ClassNode *node;

  i = ilength(arglist);
  ASSERT((i == 1), arglist, WNA, "SCM_CPlusPlus_ObjectP");
  object = CAR(arglist);

  if (!CELLP(object))
    return BOOL_F;

  if (!((CAR(object) & 127) == tc7_smob))
    return BOOL_F;

  object_type = TYP32(object);
  node = classController->binarySearchByType(object_type);
  if (!node)
    {
      return BOOL_F;
    }
  return BOOL_T;
}  

iprocCFunction *
iprocBinarySearch(iproc *iprocs,
		  char *method_name,
		  int min,
		  int max)
{
  int median;
  int result;

 repeat:
  if (max < min)
    return NULL;

  ;if (min == max)
    {
      if (strcmp(method_name, iprocs[min].string) == 0)
	return iprocs[min].cproc;
      else
	return NULL;
    }

  median = (min + max) >> 1;

  result = strcmp(method_name, iprocs[median].string);

  if (result < 0)
    {
      max = median - 1;
      goto repeat;
    }
  else
    {
      if (result > 0)
	{
	  min = median + 1;
	  goto repeat;
	}
      else
	{
	  return iprocs[median].cproc;
	}
    }
}

SCM
SCM_Mutate_float(SCM obj, float f)
{
  ASSERT(CELLP(obj), obj, ARG2, "SCM_Mutate_float");
  ASSERT(INEXP(obj), obj, ARG2, "SCM_Mutate_float");
  (*((dbl *) SCM2PTR(obj))->real) = (double) f;
  return UNSPECIFIED;
}

void
init_SCM_Global_Dispatch()
{
  classController = new ClassController;
  hashTable = HashTableCreate(2, NULL);
  init_iprocs(list_subrs, tc7_lsubr);
}
