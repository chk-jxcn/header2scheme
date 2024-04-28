/*
 * StringList.c++
 * Author: Kenneth B. Russell <kbrussel@media.mit.edu>
 * Copyright 1995, Kenneth B. Russell and the MIT Media Lab.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <iostream.h>
#include "StringList.h"

#ifdef __sgi
#include <Inventor/fields/SoMFString.h>
#endif

StringList::StringList(int num_strings ...)
{
  int counter;
  char *cptr;
  StringListStruct *sptr = NULL;
  
  strs = NULL;
  va_list args;
  va_start(args, num_strings);

  for (counter = 0; counter <= num_strings; counter++)
    {
      cptr = va_arg(args, char*);
      cerr << "cptr was " << cptr << endl;
      fflush(stderr);
      if (sptr)
	{
	  sptr->next = new StringListStruct;
	  sptr = sptr->next;
	  sptr->next = NULL;
	  sptr->string = new char[strlen(cptr) + 1];
	  strcpy(sptr->string, cptr);
	}
      else
	{
	  strs = new StringListStruct;
	  sptr = strs;
	  sptr->next = NULL;
	  sptr->string = new char[strlen(cptr) + 1];
	  strcpy(sptr->string, cptr);
	}
    }
  
  va_end(args);
}

StringList::StringList()
{
  strs = NULL;
}
  
StringList::~StringList()
{
  if (strs)
    {
      while (strs)
	{
	  delete[] strs->string;
	  delete strs;
	  strs = strs->next;
	}
    }
}

void
StringList::setStrings(int num_strings ...)
{
  int counter;
  char *cptr;
  StringListStruct *sptr = NULL;

  if (strs)
    {
      while (strs)
	{
	  delete[] strs->string;
	  delete strs;
	  strs = strs->next;
	}
    }

  strs = NULL;
  va_list args;
  va_start(args, num_strings);

  for (counter = 0; counter < num_strings; counter++)
    {
      cptr = va_arg(args, char*);
      if (sptr)
	{
	  sptr->next = new StringListStruct;
	  sptr = sptr->next;
	  sptr->next = NULL;
	  sptr->string = new char[strlen(cptr) + 1];
	  strcpy(sptr->string, cptr);
	}
      else
	{
	  strs = new StringListStruct;
	  sptr = strs;
	  sptr->next = NULL;
	  sptr->string = new char[strlen(cptr) + 1];
	  strcpy(sptr->string, cptr);
	}
    }
  
  va_end(args);
}

#ifdef __sgi
void
StringList::setStringsFromSoMFString(const SoMFString& strings,
				     int start_index)
{
  int counter = start_index;
  int temp_length;
  StringListStruct *sptr = NULL;

  if (strs)
    {
      while (strs)
	{
	  delete[] strs->string;
	  delete strs;
	  strs = strs->next;
	}
    }

  for (counter = start_index; 
       counter < strings.getNum(); 
       counter++)
    {
//      cerr << "Set string: " << strings[counter].getString() << endl;
      if (sptr)
	{
	  sptr->next = new StringListStruct;
	  sptr = sptr->next;
	  sptr->next = NULL;
	  temp_length = strings[counter].getLength();
	  sptr->string = new char[temp_length + 1];
	  strcpy(sptr->string, strings[counter].getString());
	}
      else
	{
	  strs = new StringListStruct;
	  sptr = strs;
	  sptr->next = NULL;
	  temp_length = strings[counter].getLength();
	  sptr->string = new char[temp_length + 1];
	  strcpy(sptr->string, strings[counter].getString());
	}
    }
}
#endif

void
StringList::buildStringListFromFile(char *filename)
{
  FILE *fp;
  StringListStruct *sptr;
  char in_string[255];
 
  if (strs)
    {
      while (strs)
	{
	  delete[] strs->string;
	  delete strs;
	  strs = strs->next;
	}
    }

  strs = NULL;
  fp = fopen(filename, "r");

  while (fgets(in_string, 255, fp))
    {
      if (!(in_string[0] == '\n' || in_string[0] == ';'))  // may as well
							   // filter blank
							   // lines and
							   // comments
	{
	  if (strs == NULL)
	    {
	      strs = new StringListStruct;
	      sptr = strs;
	      sptr->next = NULL;
	      sptr->string = strdup(in_string);
	    }
	  else
	    {
	      sptr->next = new StringListStruct;
	      sptr = sptr->next;
	      sptr->string = strdup(in_string);
	      sptr->next = NULL;
	    }
	  cout << "Line read: " << sptr->string << endl;
	}
    }
  fclose(fp);
}

