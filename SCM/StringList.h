/*
 * StringList.h++
 * Author: Kenneth B. Russell <kbrussel@media.mit.edu>
 * Copyright 1995, Kenneth B. Russell and the MIT Media Lab.
 */

#ifndef __STRINGLIST__
#define __STRINGLIST__

#include "StringListStruct.h"

#ifdef __sgi
class SoMFString;
#endif

class StringList
{
 private:
  StringListStruct *strs;

  friend class Scheme;
 public:
  StringList();
  StringList(int num_strings, ...);
  ~StringList();
  void setStrings(int num_strings, ...);
#ifdef __sgi
  void setStringsFromSoMFString(const SoMFString& strings,
				int start_index);
#endif
  void buildStringListFromFile(char *filename);
};
    
#endif
