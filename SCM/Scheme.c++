/*
 * Scheme.c++
 * Author: Kenneth B. Russell <kbrussel@media.mit.edu>
 * Copyright 1995, Kenneth B. Russell and the MIT Media Lab.
 */

#include <stdio.h>
#include <iostream.h>
#include <bstring.h>
#include <string.h>
#include <stdarg.h>

#include "Scheme.h"

extern int verbose;

extern "C" {
SCM mkstrport(SCM, long);
SCM set_inp(SCM);
}

extern void restore_signals();

SCM Scheme::env = (SCM)EOL;

Scheme::Scheme()
{
}

void
Scheme::initScheme(int user_verbose)

{
  init_scm(user_verbose);
}

void
Scheme::initArgs(int argc, char **argv)
{
  SCM i;

  // Copied from scm.c++, run_scm

  progargs = EOL;
  i = argc;
  while (i--)
    progargs = cons(makfromstr(argv[i],
			       (sizet)strlen(argv[i])), progargs);
}

void
Scheme::initFile(char *init_file_name)
{
  init_init(init_file_name);
}

/*
SCM
Scheme::evalSchemeOld(StringList *strlist)
{
  SCM retval;

  retval = repl_from_strlist(strlist->strs, env);
  return retval;
}
*/
SCM
Scheme::evalScheme(StringList *strlist)
{
  char *evalbuf;
  int lengthcounter = 0, linecounter = 0;
  StringListStruct *sp;

  if (strlist)
    sp = strlist->strs;
  else
    return (SCM)NULL;

  while (sp)
    {
      lengthcounter += strlen(sp->string);
      linecounter++;
      sp = sp->next;
    }

  evalbuf = new char[lengthcounter + linecounter + 1];
  evalbuf[0] = 0;
  sp = strlist->strs;
  while(sp)
    {
      strcat(evalbuf, sp->string);
      if (sp->next)
	strcat(evalbuf, " ");
      sp = sp->next;
    }

  SCM p = mkstrport(cons(INUM0,makfromstr(evalbuf, strlen(evalbuf))),OPN | RDNG);
  SCM oldinputport;
  oldinputport = set_inp(p);
  initSignals();
  SCM retval = repl_driver_once_with_return();
  restoreSignals();
  set_inp(oldinputport);
  delete[] evalbuf;
  return retval;
}

SCM
Scheme::callSchemeFunc(char *funcname,
		       int num_args ...)
{
  SCM tok_buf;
  SCM x, tmp, cons_ptr;
  
  long length;
  int counter;

  // First get the SCM object associated with the function name

  length = (long) strlen(funcname);
  tok_buf = makstr(length);
  bcopy(funcname, CHARS(tok_buf), length);
  tmp = intern(CHARS(tok_buf), length);
  x = cons_ptr = cons(CAR(tmp), EOL);

  // Now cons up the list of arguments

  va_list args;
  va_start(args, num_args);

  for (counter = 0; counter < num_args; counter++)
    {
      tmp = va_arg(args, SCM);
      cons_ptr = (CDR(cons_ptr) = cons(tmp,EOL));
    }

  // Call SCM with the completed function
  initSignals();
  x = repl_driver_once_with_return_and_eval(x);
  restoreSignals();
  if (verbose > 2)
    {
      iprin1(x,cur_outp,1);
      lputc('\n',cur_outp);
    }
  // return Scheme object to C++
  return x;
}

void
Scheme::evalOnceFromStdin()
{
  initSignals();
  repl_driver_once();
  restoreSignals();
}

void
Scheme::initSignals()
{
  init_signals();
}

void
Scheme::restoreSignals()
{
  restore_signals();
}
