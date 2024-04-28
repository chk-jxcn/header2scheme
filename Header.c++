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

#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include "Header.h"
#include "DirectoryTree.h"

#define BIG_DEBUG 0

const int EOL = 10;

Header::Header()
{
  classes = new SLList<Class *>;
  file = new File;

  filename = NULL;
  name = NULL;
}

Header::Header(const char* origfilename)
{
  classes = new SLList<Class *>;
  file = new File;

  filename = new char[strlen(origfilename) + 1];
  strcpy(filename, origfilename);
  name = stringWithoutSuffix(origfilename);
}

Header::~Header()
{
  if (filename) delete[] filename;
  if (name) delete[] name;
  Pix placeholder;
  placeholder = classes->first();
  while (placeholder)
    {
      Class *cla = (*classes)(placeholder);
      delete cla;
      classes->next(placeholder);
    }
  delete classes;
  delete file;
}

void
Header::setDirectoryTree(DirectoryTree *dt)
{
  parent_directory = dt;
}

void
Header::setFileName(const char *newfilename)
{
  if (filename)
    delete[] filename;
  if (name)
    delete[] name;

  filename = new char[strlen(newfilename) + 1];
  strcpy(filename, newfilename);
  name = stringWithoutSuffix(newfilename);
}

char *
Header::stringWithoutSuffix(const char* filename)
{
  int newlength;

  newlength = strlen(filename);
  
  while (filename[newlength - 1] != '.')
    {
      newlength--;
    }

  char *newname = new char[newlength];

  strncpy(newname, filename, newlength - 1);
  newname[newlength - 1] = 0;

  return newname;
}

void
Header::readFromFile()
{
  FILE *fp;
  int c, d;
  char temp_tok_buf[255];
  int counter = 0;

  // Read the entire file into the File data structure.
  // Then call buildClassList on it.

  if (parent_directory->usingMacros())
    {
      fp = popen(parent_directory->getMacroCommand(filename), "r");
    }
  else
    {
      fp = fopen(filename, "r");
    }
  if (fp == NULL)
    {
      cerr << "Header::readFromFile: Error opening file " << filename << " for input" << endl;
      return;
    }

  while ((c = getc(fp)) != EOF)
    {
#if BIG_DEBUG
      cerr << "char1: " << (char)c << endl;
#endif

      if (c == '/')
	{
	  // possibly get ready to read a comment
	  d = getc(fp);
#if BIG_DEBUG
      cerr << "char2: " << (char)d << endl;
#endif
	  switch (d)
	    {
	    case '*':
	      {
		// full comment. Read until next '*/'
		int done = 0;
		do
		  {
		    c = getc(fp);
#if BIG_DEBUG
      cerr << "char3: " << (char)c << endl;
#endif
		    if (c == '*')
		      {
			c = getc(fp);
#if BIG_DEBUG
      cerr << "char4: " << (char)c << endl;
#endif
			if (c == '/')
			  done = 1;
			else
			  ungetc(c, fp);
		      }
		  }
		while ((!done) && (c != EOF));
		if (c == EOF)
		  {
		    cerr << "Header::readFromFile: malformed comment" << endl;
		  }
		break;
	      }

	    case '/':
	      {
		// comment until end of line only
		int done = 0;
		do
		  {
		    c = getc(fp);
#if BIG_DEBUG
      cerr << "char5: " << (char)c << endl;
#endif
		    if (c == EOL)
		      done = 1;
		  }
		while ((!done) && (c != EOF));
		if (c == EOF)
		  {
		    cerr << "Header::readFromFile: malformed comment" << endl;
		  }
		break;
	      }
	      
	    default:
	      {
		// must not be a comment
		ungetc(d, fp);
		temp_tok_buf[0] = c;
		temp_tok_buf[1] = 0;
		file->addToken(temp_tok_buf);
	      }
	    }
	}
      else
	{
	  if (LETTER(c))
	    {
	      ungetc(c, fp);
	      int done = 0;
	      do
		{
		  c = getc(fp);
#if BIG_DEBUG
      cerr << "char6: " << (char)c << endl;
#endif
		  if (c == ':')  // check for scope operator in variable type
		    {
		      char e;
		      e = getc(fp);
#if BIG_DEBUG
      cerr << "char7: " << (char)e << endl;
#endif
		      if (e == ':')
			{
			  temp_tok_buf[counter++] = c;
			  temp_tok_buf[counter++] = e;
			}
		      else
			{
			  ungetc(e, fp);
			  done = 1;
			}
		    }
		  else
		    if (LETTERORNUMBER(c))
		      temp_tok_buf[counter++] = c;
		    else
		      done = 1;
		}
	      while (!done);
	      ungetc(c, fp);
	      temp_tok_buf[counter] = 0;
	      counter = 0;
#if BIG_DEBUG
	      cerr << "added token \"" << temp_tok_buf << "\" to file 1" << endl;
#endif
	      file->addToken(temp_tok_buf);
	    }
	  else
	    {
	      if (c > 32)  // i.e. visible ASCII characters
		{
		  temp_tok_buf[0] = c;
		  temp_tok_buf[1] = 0;
#if BIG_DEBUG
		  cerr << "added token \"" << temp_tok_buf << "\" to file 2" << endl;
#endif
		  file->addToken(temp_tok_buf);
		}
	    }
	}
    }
  
  if (parent_directory->usingMacros())
    {
      pclose(fp);
    }
  else
    {
      fclose(fp);
    }
  buildClassList();
}      

void
Header::buildClassList()
{
  Pix placeholder;
  char *token;
  int done = 0;
  int classdone;

  Pix p1;
  char *temptok;

  // for checking whether "public" is actually
  // SoINTERNAL public: or SoEXTENDER public:
  // as well as whether "class" is actually
  // SoINTERNAL class or SoEXTENDER class
  Pix p2, p3;
  char *intok;

  Line *line;

  // start at the beginning
  placeholder = file->tokens->first();
  token = (*(file->tokens))(placeholder);
  
  while (!done)
    {
    repeat:
      // scan for a class
      while (placeholder && (strcmp(token, "class") != 0))
	{
	  file->tokens->next(placeholder);
	  if (placeholder)
	    token = (*(file->tokens))(placeholder);
	  else
	    {
	      done = 1;
	    }
	}

      if (!done)
	{
	  // check against forward declarations

	  p1 = placeholder;
	  file->tokens->next(p1);
	  file->tokens->next(p1);
	  if (p1)
	    {
	      temptok = (*(file->tokens))(p1);
	      if (strcmp(temptok, ";") == 0)
		{
		  file->tokens->next(p1);
		  placeholder = p1;
		  token = (*(file->tokens))(placeholder);
		  goto repeat;
		}
	    }
	  else
	    done = 1;
	}

/* Following code will not work;
   need to skip to the end of the class and
   not add this class to the class list of this header.
   I'll probably do this elsewhere in the Ignores
   (i.e. be able to ignore a class).
*/

/*
      if (!done)
	{
	  // got a class definition.
	  // make sure it's not a SoINTERNAL class
	  // or an SoEXTENDER class.

	  p2 = NULL;
	  p3 = file->tokens->first();
	  while ((p3 != placeholder) && p3)
	    {
	      p2 = p3;
	      file->tokens->next(p3);
	    }
	      
	  if (p2)
	    {
	      intok = (*(file->tokens))(p2);
	      if (!strcmp(intok, "SoINTERNAL"))
		{
		  cerr << "Skipping SoINTERNAL class \"" << temp_class->name << "\"" << endl;
		  goto repeat;
		}

	      if (!strcmp(intok, "SoEXTENDER"))
		{
		  cerr << "Skipping SoEXTENDER class \"" << temp_class->name << "\"" << endl;
		  goto repeat;
		}
	    }
	}
*/
      if (!done)
	{
	  // got a class definition.
	  Class *temp_class = new Class;
	  temp_class->setHeader(this);
	  int bracecounter = 0;

	  // get the class name
	  file->tokens->next(placeholder);
	  temp_class->name = new char[strlen((*(file->tokens))(placeholder)) + 1];
	  strcpy(temp_class->name, (*(file->tokens))(placeholder));

	  // extract this class's parent classes if it has any
	  do
	    {
	      file->tokens->next(placeholder);
	      token = (*(file->tokens))(placeholder);
	      if (strcmp(token, "public") == 0)
		{
		  file->tokens->next(placeholder);
		  token = (*(file->tokens))(placeholder);
		  char *supbuf = new char[strlen(token)+1];
		  strcpy(supbuf, token);
		  temp_class->superclasses->append(supbuf);
		}
#if BIG_DEBUG
	      cerr << "token: " << token << endl;
#endif
	    }
	  while (strcmp(token, "{") != 0);

	skip_to_public:
	  // skip ahead to the start of the public interface of the class (ignoring derivations)
	  bracecounter = 1;
	  do
	    {
	      file->tokens->next(placeholder);
	      token = (*(file->tokens))(placeholder);
	      if (strcmp(token, "{") == 0)
		{
		  bracecounter++;
//		  cerr << "bracecounter was incremented to " << bracecounter << endl;
		}
	      if (strcmp(token, "}") == 0)
		{
		  bracecounter--;
//		  cerr << "bracecounter was decremented to " << bracecounter << endl;
		}
#if BIG_DEBUG
	      cerr << "token: " << token << endl;
#endif
	    }
	  while ((strcmp(token, "public") != 0) && (bracecounter > 0));

	check_public:
	  if (strcmp(token, "public") == 0)
	    {
	      // check to make sure this isn't an SoINTERNAL or SoEXTENDER
	      // portion of this class.

	      p3 = file->tokens->first();
	      while ((p3 != placeholder) && p3)
		{
		  p2 = p3;
		  file->tokens->next(p3);
		}
	      
//	      file->tokens->next(p2);

	      if (!p2 || !p3)
		{
		  cerr << "Header::buildClassList: Error while checking \"public\"" << endl;
		}
	      else
		{
		  intok = (*(file->tokens))(p2);
		  if (!strcmp(intok, "SoINTERNAL"))
		    {
//		      cerr << "Skipping SoINTERNAL portion of class \"" << temp_class->name << "\"" << endl;
		      goto skip_to_public;
		    }

		  if (!strcmp(intok, "SoEXTENDER"))
		    {
//		      cerr << "Skipping SoEXTENDER portion of class \"" << temp_class->name << "\"" << endl;
		      goto skip_to_public;
		    }
		}
	    }

	  if (bracecounter == 0)
	    {
	      classdone = 1;
#if BIG_DEBUG
	      cerr << "didn't find another public interface" << endl;
#endif
	      goto skipclass;
	    }

	  file->tokens->next(placeholder);
	  token = (*(file->tokens))(placeholder);

	  // now start reading off the lines until we reach the
	  // end of the public interface.

	  classdone = 0;

	nextline:
	  do
	    {
	      line = new Line;

	      // copy tokens into this line until we reach the end.
	      // looking for a ';' if it's a "variable" line,
	      // or for a ';' or { ... } group if it's a "method" line.
	      // assume it's a variable line at first

	      do
		{
		  file->tokens->next(placeholder);
		  token = (*(file->tokens))(placeholder);
		  
		  if (strcmp(token, "}") == 0)
		    {
		      delete line;
		      classdone = 1;
//		      cerr << "going to skipclass" << endl;
		      goto skipclass;
		    }

		  if (strcmp(token, "private") == 0 || 
		      strcmp(token, "protected") == 0) /* skip ahead to next
							  public section (if any) */
		    {
//		      cerr << "Skipping to next public section in class \"" << temp_class->name << "\"" << endl;
		      delete line;
		      goto skip_to_public;
		    }

		  if (strcmp(token, "public") == 0)  /* superfluous "public:",
							possibly. Want to check
							to make sure it isn't
							a SoINTERNAL public:
							or SoEXTENDER public: */
		    {
		      delete line;
		      goto check_public;
//		      cerr << "Superfluous \"public:\" in class \"" << temp_class->name << "\"" << endl;
//		      file->tokens->next(placeholder);
//		      goto nextline;
		    }

		  if (strcmp(token, "enum") == 0)  // enumeration definition, ignore
		    {
		      while (placeholder && strcmp(token, ";") != 0)
			{
			  file->tokens->next(placeholder);
			  token = (*(file->tokens))(placeholder);
			}
		      goto nextline;
		    }
		  else if (strcmp(token, "typedef") == 0) // typedef, ignore
		    {
		      while (placeholder && strcmp(token, ";") != 0)
			{
			  file->tokens->next(placeholder);
			  token = (*(file->tokens))(placeholder);
			}
		      goto nextline;
		    }
		  else
		    {
		      if (strcmp(token, "(") == 0)
			{
			  line->type = METHOD;
			}
		      
#if BIG_DEBUG
		      cerr << "added token " << token << endl;
#endif
		      line->addToken(token);
		    }
		}
	      while ((line->type == VARIABLE) ? (strcmp(token, ";") != 0) : (strcmp(token, ";") != 0 && strcmp(token, "{") != 0));
#if BIG_DEBUG
	      cerr << "foo" << endl;
#endif
	      
	      // flush out inline function definitions if necessary
	      if (line->type == METHOD)
		{
		  int bracecount = 0;

		  if (strcmp(token, "{") == 0)  // flush out definition
		    {
		      bracecount = 1;
		  
		      do
			{
			  file->tokens->next(placeholder);
			  token = (*(file->tokens))(placeholder);
			  if (strcmp(token, "{") == 0)
			    {
			      bracecount += 1;
			    }
			  if (strcmp(token, "}") == 0)
			    bracecount -= 1;
			}
		      while (bracecount > 0);
		      
		      Pix p1 = placeholder;
		      char *temptok;
		      file->tokens->next(p1);
		      temptok = (*(file->tokens))(p1);
		      if (strcmp(temptok, ";") == 0)   // optional semicolon after inline function definition
			placeholder = p1;
		      token = temptok;
		    }
		}
	      
	      temp_class->addLine(line);

	      goto endline;
	    skipclass:   /* should only be called if we need 
			    to skip to the end of a class */
	      {
		if (strcmp(token, "}") != 0)
		  {
		    // need to skip to the end of the class.
		    int bracecounter2 = 1;
		    file->tokens->next(placeholder);
		    while (placeholder && (bracecounter2 > 0))
		      {
			char *tok = (*(file->tokens))(placeholder);
			if (strcmp(tok, "{") == 0)
			  bracecounter2++;
			if (strcmp(tok, "}") == 0)
			  bracecounter2--;
			file->tokens->next(placeholder);
		      }
		  }
	      }
	    endline: {}
	    }
	  while (!classdone);
	  
//	  cerr << "Adding class \"" << temp_class->name << "\" to header" << endl;
	  appendToClasses(temp_class);
//	  classes->append(temp_class);
	}
    }
}  

void
Header::appendToClasses(Class *a_class)
{
  if (!parent_directory->classInIgnoresList(a_class->name))
    classes->append(a_class);
}

void
Header::print()
{
  Class *tempclass;
  Pix placeholder;

  cout << "Filename: " << filename << " Name: " << name << endl;

  placeholder = classes->first();

  while (placeholder)
    {
      tempclass = (*classes)(placeholder);
      tempclass->print();
      classes->next(placeholder);
    }
  cout << "****" << endl;
}

void
File::printTokens()
{
  char *tok_buf;
  Pix placeholder;

  placeholder = tokens->first();
  tok_buf = (*tokens)(placeholder);
  cout << " " << tok_buf;

  do
    {
      tokens->next(placeholder);
      if (placeholder != 0)
	{
	  tok_buf = (*tokens)(placeholder);
	  cout << " " << tok_buf;
	}
    }
  while (placeholder != 0);
}

int
isNonEssentialKeyword(char *token)
{
  if (strcmp(token, "register") == 0 ||
      strcmp(token, "virtual") == 0 ||
      strcmp(token, "unsigned") == 0 ||
      strcmp(token, "inline") == 0 ||
      strcmp(token, "volatile") == 0)
    return 1;

  if (strcmp(token, "const") == 0)
    return 2;

  if (strcmp(token, "static") == 0)
    return 3;
    
  if (strcmp(token, "friend") == 0)
    return 4;

  if (strcmp(token, "long") == 0)
    return 5;

  if (strcmp(token, "short") == 0)
    return 6;

  if (strcmp(token, "struct") == 0)
    return 7;

  if (strcmp(token, "SoINTERNAL") == 0)
    {
      cerr << "Skipping SoINTERNAL" << endl;
      return 1;
    }

  if (strcmp(token, "SoEXTENDER") == 0)
    {
      cerr << "Skipping SoEXTENDER" << endl;
      return 1;
    }

	
  return 0;
}

int
operatorLookup(char *token)
{
  int counter;

  for (counter = 0; counter < 40; counter++)
    {
      if (strcmp(token, OperatorList[counter]) == 0)
	return (counter + 1);
    }
  cerr << "Operator token " << token << " not found" << endl;
  return 0;
}
