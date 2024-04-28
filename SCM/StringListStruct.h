/*
 * StringListStruct.h
 * Author: Kenneth B. Russell <kbrussel@media.mit.edu>
 * Copyright 1995, Kenneth B. Russell and the MIT Media Lab.
 */

#ifndef __STRINGLISTSTRUCT__
#define __STRINGLISTSTRUCT__

typedef struct StringListStruct_t {
  char *string;
  struct StringListStruct_t *next;
} StringListStruct;

#endif
