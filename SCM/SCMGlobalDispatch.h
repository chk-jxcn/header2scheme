#ifndef _SCM_GLOBAL_DISPATCH_
#define _SCM_GLOBAL_DISPATCH_

extern "C" {
#include "scm.h"
}
#include "ClassController.h"
#include "hash.h"

extern ClassController *classController;
extern HashTable hashTable;

// prototype for global dispatch routine
SCM SCM_Global_Dispatch(SCM arglist, ...);

// prototype for C++ object predicate routine in Scheme
SCM SCM_CPlusPlus_ObjectP(SCM arglist, ...);

// prototype for iproc binary search routine
typedef SCM  iprocCFunction(SCM, ...);
iprocCFunction *iprocBinarySearch(iproc *iprocs, char *method_name, int min, int max);

// convenience function for mutation of floating point values in SCM
SCM SCM_Mutate_float(SCM obj, float f);

// function to initialize the global dispatch routine
void init_SCM_Global_Dispatch();

#endif

