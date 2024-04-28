/*
 * Scheme.h
 * Author: Kenneth B. Russell <kbrussel@media.mit.edu>
 * Copyright 1995, Kenneth B. Russell and the MIT Media Lab.
 */

#ifndef __SCHEMECLASS__
#define __SCHEMECLASS__

extern "C" {
#include "scm.h"
#include "setjump.h"
}

#define SCHEME_SET_GC_BASE int scheme_gc_base; BASE(rootcont) = (STACKITEM *)&scheme_gc_base

#include "StringList.h"

class Scheme {
 public:
  static SCM env;
  
  // initializer function for SCM
  static void initScheme(int user_verbose);

  // initializer for preparing SCM's notion of the program arguments

  static void initArgs(int argc, char **argv);

  // function to manually load an initialization file into the interpreter
  static void initFile(char *init_file_name);

  // evaluate the data structure of scheme code
  static SCM evalScheme(StringList *strlist);

  // method to call a Scheme function with arguments
  // generated in C++
  static SCM callSchemeFunc(char *funcname, int num_args, ...);

  // method to call the read-eval-print "non-loop" once
  static void evalOnceFromStdin();

protected:
  Scheme();  // should never call constructor
  ~Scheme();

  static void initSignals();
  static void restoreSignals();
};
    
#endif
