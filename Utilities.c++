/*
 * Header2Scheme -- C++ Foreign Function Interface Generator for Scheme.
 * Copyright (C) 1995 Kenneth B. Russell <kbrussel@media.mit.edu>
 * Kenneth B. Russell, 20 Ames Street (E15-394), Cambridge, MA 02139.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "Utilities.h"

char **
SLListToCharArray(SLList <char *> *list)
{
  char **ptr;
  Pix placeholder;
  char *string;
  int counter = 0;
  
  ptr = new char *[list->length() + 1];
  placeholder = list->first();
  while (placeholder)
    {
      string = (*list)(placeholder);
      ptr[counter++] = string;
      list->next(placeholder);
    }
  ptr[counter] = 0;
  return ptr;
}

int
conditionalAddStringToSLList(char *string, SLList<char *> &list)
{
  Pix p1;
  char *comp;
  int addflag = 1;
  
  p1 = list.first();
  while (p1 && addflag)
    {
      comp = list(p1);
      if (strcmp(comp, string) == 0)
	addflag = 0;
      else
	list.next(p1);
    }
  if (addflag)
    {
      list.append(string);
      return 0;
    }
  return -1;
}

char *
stringToUppercase(char *string)
{
  char *upperstring;

  upperstring = new char[strlen(string) + 1];
  int counter = 0;
  while (counter < strlen(string))
    {
      upperstring[counter] = toupper(string[counter]);
      counter++;
    }
  upperstring[counter] = 0;
  return upperstring;
}

char *
stringToLowercase(char *string)
{
  char *lowerstring;

  lowerstring = new char[strlen(string) + 1];
  int counter = 0;
  while (counter < strlen(string))
    {
      lowerstring[counter] = tolower(string[counter]);
      counter++;
    }
  lowerstring[counter] = 0;
  return lowerstring;
}

