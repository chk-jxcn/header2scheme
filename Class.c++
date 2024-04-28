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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "Header.h"
#include "DirectoryTree.h"
#include "Utilities.h"

#define BIG_DEBUG 0

Class::Class()
{
  name = NULL;
  superclasses = new SLList<char *>;
  methods = new SLList<Method *>;
  variables = new SLList<Variable *>;
  exists_as_pointer_only = 0;
}

Class::~Class()
{
  if (name) delete[] name;
  Pix placeholder;
  placeholder = superclasses->first();
  while (placeholder)
    {
      char *sup = (*superclasses)(placeholder);
      delete[] sup;
      superclasses->next(placeholder);
    }
  delete superclasses;
  placeholder = methods->first();
  while (placeholder)
    {
      Method *met = (*methods)(placeholder);
      delete met;
      methods->next(placeholder);
    }
  delete methods;
  placeholder = variables->first();
  while (placeholder)
    {
      Variable *var = (*variables)(placeholder);
      delete var;
      variables->next(placeholder);
    }
  delete variables; 
}

void
Class::setHeader(Header *hd)
{
  parent_header = hd;
}

void
Class::addLine(Line *line)
{
  if (line->type == VARIABLE)
    {
      Variable *variable = new Variable;
      Pix placeholder;
      char *token;

      // get rid of all non-essential keywords and extract the base type.

      placeholder = line->tokens->first();
      token = (*(line->tokens))(placeholder);

      int is_static;

      while (is_static = isNonEssentialKeyword(token))
	{
	  if (is_static == 3)  // static member variable
	    {
	      variable->is_static = 1;
	    }
	  if (is_static == 2)  // const member variable
	    {
	      variable->is_constant = 1;
	    }
	  if (is_static == 4)  // "friend class foo;", ignore this
	    {
	      delete variable;
	      return;
	    }
	  if (is_static == 5)  // "long"
	    {
	      variable->td->is_long = 1;
	    }
	  if (is_static == 6)  // "short"
	    {
	      variable->td->is_short = 1;
	    }
	  if (is_static == 7)  // "struct"
	    {
	      variable->td->is_struct = 1;
	    }

	  line->tokens->next(placeholder);
	  token = (*(line->tokens))(placeholder);
	}
	  
      if ((strcmp(token, "*") == 0) || strcmp(token, "&") == 0)
	{
	  // base type *was* "short", "long", etc.
	  // implies the base type is an int

	  variable->td->type = new char[4];
	  strcpy(variable->td->type, "int");
	}
      else
	{
	  variable->td->type = new char[strlen(token) + 1];
	  strcpy(variable->td->type, token);

	  line->tokens->next(placeholder);
	  token = (*(line->tokens))(placeholder);
	}

      // now we should be looking either at the variable name
      // or an indication of whether this variable is a pointer.

      if (strcmp(token, "*") == 0)
	{
	  variable->td->ref = POINTER;
	  do
	    {
	      variable->td->depth += 1;
	      line->tokens->next(placeholder);
	      token = (*(line->tokens))(placeholder);
	    }
	  while (strcmp(token, "*") == 0);
	}

      if (strcmp(token, ";") == 0 || 
	  strcmp(token, "[") == 0 ||
	  strcmp(token, "&") == 0)    // oops...type should have been int
	{
	  variable->name = new char[strlen(variable->td->type) + 1];
	  strcpy(variable->name, variable->td->type);
	  delete[] variable->td->type;
	  variable->td->type = new char[4];
	  strcpy(variable->td->type, "int");
	}
      else
	{
//	  variable->name = new char[strlen(token) + 1];
//	  strcpy(variable->name, token);
	  variable->name = conditionalUnscopeName(token);
	}

      int array_dimension_index = 0;

      while (strcmp(token, ";") != 0 && strcmp(token, "&") != 0)
	{
	arrayrepeat:
	  if (strcmp(token, "[") == 0)
	    {
	      line->tokens->next(placeholder);
	      token = (*(line->tokens))(placeholder);

	      if (strcmp(token, "]") == 0)
		{
		  variable->td->is_array = array_dimension_index + 1;
		  variable->td->array_dimensions[array_dimension_index] = 0;
		}
	      else
		{
		  if (NUMBER(token[0]))
		    {
		      variable->td->is_array = array_dimension_index + 1;
		      variable->td->array_dimensions[array_dimension_index] = atoi(token);

		      line->tokens->next(placeholder);
		      token = (*(line->tokens))(placeholder);
		      while (NUMBER(token[0]))
			{
			  variable->td->array_dimensions[array_dimension_index] *= 10;
			  variable->td->array_dimensions[array_dimension_index] += atoi(token);

			  line->tokens->next(placeholder);
			  token = (*(line->tokens))(placeholder);
			}
		      goto arrayrepeat;
		    }
		  else
		    {
		      cerr << "Error in class " << name << ": invalid array dimension: " << token << endl;
		    }
		}
	      array_dimension_index++;
	    }

	  line->tokens->next(placeholder);
	  token = (*(line->tokens))(placeholder);
	}
      variables->append(variable);
    }
  else
    {
      Method *method = new Method;
      Pix placeholder;
      char *token;
      int is_static = 0;

      // get rid of all non-essential keywords and extract the base type of
      // the return value of the function.

      placeholder = line->tokens->first();
      token = (*(line->tokens))(placeholder);

      while (is_static = isNonEssentialKeyword(token))
	{
#if BIG_DEBUG
		  cerr << "nonessential token was " << token << endl;
#endif
	  if (is_static == 3)
	    {
	      method->is_static = 1;
	    }
	  if (is_static == 2) // "const"
	    {
	      method->is_constant = 1;
	    }
	  if (is_static == 4) /* "friend" function.
				 We'll parse it as usual,
				 and ignore it at the end
				 if it isn't an operator function.
				 This may change. */
	    
	    {
	      method->is_friend = 1;
	    }
	  if (is_static == 5) // "long"
	    {
	      method->td->is_long = 1;
	    }
	  if (is_static == 6) // "short"
	    {
	      method->td->is_short = 1;
	    }
	  if (is_static == 7) // "struct"
	    {
	      method->td->is_struct = 1;
	    }

	  line->tokens->next(placeholder);
	  token = (*(line->tokens))(placeholder);
	}
      
      if ((strcmp(token, "*") == 0) || (strcmp(token, "&") == 0))
	{
	  // base type was "long", "short", etc.
	  // implies base type of int

	  method->td->type = new char[4];
	  strcpy(method->td->type, "int");
	}
      else if (method->td->is_long || method->td->is_short)
	{
	  // base type was "long", "short", etc.
	  // check to see if base type is "float" or "int"

	  if (!strcmp(token, "float") || !strcmp(token, "double"))
	    {
	      method->td->type = new char[6];
	      strcpy(method->td->type, "float");
	      line->tokens->next(placeholder);
	      token = (*(line->tokens))(placeholder);
	    }
	  else if (!strcmp(token, "int"))
	    {
	      method->td->type = new char[4];
	      strcpy(method->td->type, "int");
	      line->tokens->next(placeholder);
	      token = (*(line->tokens))(placeholder);
	    }
	  else
	    {
	      method->td->type = new char[4];
	      strcpy(method->td->type, "int");
	    }
	}
      else
	{
	  method->td->type = new char[strlen(token) + 1];
	  strcpy(method->td->type, token);

	  line->tokens->next(placeholder);
	  token = (*(line->tokens))(placeholder);
	}

      if (strcmp(method->td->type, "operator") == 0 &&
	  (method->td->is_long == 0) && (method->td->is_short == 0))
	// definition of a cast, we can't deal with this
	{
	  cout << "WARNING: incontrovertible cast definition in class " << name << ": ";

	  placeholder = line->tokens->first();

	  while (placeholder)
	    {
	      token = (*(line->tokens))(placeholder);
	      cout << token << " ";
	      line->tokens->next(placeholder);
	    }

	  cout << "(skipping)" << endl;
	  delete method;
	  return;
	}

      // now we should be looking either at the method name
      // or an indication of whether this method returns a pointer or reference
      // (or a reference to a pointer).
      if (strcmp(token, "*") == 0)
	{
	  method->td->ref = POINTER;
	  do
	    {
	      method->td->depth += 1;
	      line->tokens->next(placeholder);
	      token = (*(line->tokens))(placeholder);
	    }
	  while (strcmp(token, "*") == 0);
	}

      if (strcmp(token, "&") == 0)
	{
	  if (method->td->ref == POINTER)
	    method->td->ref = REFERENCE_TO_POINTER;
	  else
	    method->td->ref = REFERENCE;
	  
	  line->tokens->next(placeholder);
	  token = (*(line->tokens))(placeholder);
	}

      if (strcmp(token, "(") != 0)   // not looking at a constructor
	{
	  method->name = conditionalUnscopeName(token);
//	  method->name = new char[strlen(token) + 1];
//	  strcpy(method->name, token);

	  if (strcmp(token, "operator") == 0)  // operator overloading.
	    {
	      char temp_operator[7];
	      int temp_operator_counter = 0;

	      line->tokens->next(placeholder);
	      token = (*(line->tokens))(placeholder);

	      if (strcmp(token, "(") == 0)  // operator()
		{
		  method->operator_num = operatorLookup("()");
		  line->tokens->next(placeholder);
		  line->tokens->next(placeholder);
		}
	      else
		{
		  if (LETTER(token[0]))  // operator new or delete
		    method->operator_num = operatorLookup(token);
		  else
		    {
		      while (strcmp(token, "(") != 0)
			{
			  temp_operator[temp_operator_counter] = token[0];
			  temp_operator_counter++;
			  line->tokens->next(placeholder);
			  token = (*(line->tokens))(placeholder);
			}
		      temp_operator[temp_operator_counter] = 0;
		      method->operator_num = operatorLookup(temp_operator);
		    }
		}
	    }
	  else  // not an operator we know how to deal with
	    {
	      do
		{
		  line->tokens->next(placeholder);
		  token = (*(line->tokens))(placeholder);
		  if (strcmp(token, "(") != 0)
		    cerr << "In file " << parent_header->filename << ", class " << name << ", method " << method->name << ": flushed token " << token << endl;
		}
	      while (strcmp(token, "(") != 0);
	    }
	}
      else
	{
	  if (strcmp(method->td->type, name) != 0)  
	    {
	      // means that the type was "long", "short", etc.
	      // and the method name was mistakenly taken to
	      // be the type, which should be "int"
	      // (in the case of the constructor, the "type"
	      // returned will be the name of the class)

//	      method->name = new char[strlen(method->td->type) + 1];
//	      strcpy(method->name, method->td->type);
	      method->name = conditionalUnscopeName(method->td->type);
	      if (method->name == NULL)  /* constructor defined as
					    "ClassName::ClassName"
					    within class */
		{
		  if (method->td->type) 
		    delete[] method->td->type;
		  method->td->type = new char[strlen(name) + 1];
		  strcpy(method->td->type, name);
		}
	      else
		{
		  delete[] method->td->type;
		  method->td->type = new char[4];
		  strcpy(method->td->type, "int");
		}
	    }
	}
	  
      // now we need to extract the arguments

      int arglistdone = 0;

      while (!arglistdone)
	{
	  Argument *argument = new Argument;

	  line->tokens->next(placeholder);
	  token = (*(line->tokens))(placeholder);

	  if (strcmp(token, ")") == 0)
	    arglistdone = 1;

	  if (!arglistdone)
	    {
	      int is_const;
	      while (is_const = isNonEssentialKeyword(token))
		{
#if BIG_DEBUG
		  cerr << "nonessential token was " << token << endl;
#endif

		  if (is_const == 2) // "const"
		    argument->is_constant = 1;
		  if (is_const == 5) // "long"
		    argument->td->is_long = 1;
		  if (is_const == 6) // "short"
		    argument->td->is_short = 1;
		  if (is_const == 7) // "struct"
		    argument->td->is_struct = 1;
		  line->tokens->next(placeholder);
		  token = (*(line->tokens))(placeholder);

#if BIG_DEBUG
		  cerr << "new token is " << token << endl;
#endif
		}
	      
	      // get the base type of the argument
	      
	      if (strcmp(token, "*") == 0 ||
		  strcmp(token, "&") == 0 ||
		  strcmp(token, "[") == 0 ||
		  strcmp(token, ",") == 0 ||
		  strcmp(token, ")") == 0)  // base type should be "int"
		{
		  argument->td->type = new char[4];
		  strcpy(argument->td->type, "int");
#if BIG_DEBUG
		  cerr << "made base type int" << endl;
#endif
		}
	      else
		{
		  if (argument->td->is_long || argument->td->is_short)
		    {
		      // base type was "long", "short", etc.
		      // check to see if base type is "float" or "int"
		      
		      if (!strcmp(token, "float") || !strcmp(token, "double"))
			{
			  argument->td->type = new char[6];
			  strcpy(argument->td->type, "float");
			  line->tokens->next(placeholder);
			  token = (*(line->tokens))(placeholder);
			}
		      else if (!strcmp(token, "int"))
			{
			  argument->td->type = new char[4];
			  strcpy(argument->td->type, "int");
			  line->tokens->next(placeholder);
			  token = (*(line->tokens))(placeholder);
			}
		      else
			{
			  argument->td->type = new char[4];
			  strcpy(argument->td->type, "int");
			}
		    }
		  else
		    {
		      argument->td->type = new char[strlen(token) + 1];
		      strcpy(argument->td->type, token);
		      
		      line->tokens->next(placeholder);
		      token = (*(line->tokens))(placeholder);
		    }

//		  argument->td->type = new char[strlen(token) + 1];
//		  strcpy(argument->td->type, token);
#if BIG_DEBUG
		  cerr << "set type to " << token << endl;
#endif
	      
//		  line->tokens->next(placeholder);
//		  token = (*(line->tokens))(placeholder);
		}

	      // figure out if it's a pointer or reference
	      
	      if (strcmp(token, "*") == 0)
		{
		  argument->td->ref = POINTER;
		  do
		    {
		      argument->td->depth += 1;
		      line->tokens->next(placeholder);
		      token = (*(line->tokens))(placeholder);
		    }
		  while (strcmp(token, "*") == 0);
		}

	      if (strcmp(token, "&") == 0)
		{
		  if (argument->td->ref == POINTER)
		    argument->td->ref = REFERENCE_TO_POINTER;
		  else
		    argument->td->ref = REFERENCE;
		  
		  line->tokens->next(placeholder);
		  token = (*(line->tokens))(placeholder);
		}
	      
	      // now we should be looking at the variable name.
	      // we don't care about it, but we want to know whether
	      // we got the type right.

	      // However, we need to keep in mind that the variable
	      // name is optional.

	      if (strcmp(token, "*") == 0 ||
		  strcmp(token, "&") == 0 ||
		  strcmp(token, "[") == 0 ||
		  strcmp(token, ",") == 0 ||
		  strcmp(token, ")") == 0)  // base type should be "int"?
		{
		  if (argument->td->is_long ||
		      argument->td->is_short)  /* i.e. found a "non-
						     essential" keyword
						     which was actually
						     the type */
		    {
		      delete[] argument->td->type;
		      argument->td->type = new char[4];
		      strcpy(argument->td->type, "int");
		    }
		}

	      // for arguments to methods, we only care about the type
	      // of the argument, not the name.
	      // however, we need to figure out if this variable is an
	      // array, so do so while we're skipping to the next argument.
	      
	      int array_dimension_index = 0;

	      while (strcmp(token, ",") != 0 && strcmp(token, ")") != 0)
		{
		  if (strcmp(token, "[") == 0)
		    {
		      line->tokens->next(placeholder);
		      token = (*(line->tokens))(placeholder);

		      if (strcmp(token, "]") == 0)
			{
			  argument->td->is_array = array_dimension_index + 1;
			  argument->td->array_dimensions[array_dimension_index] = 0;
			}
		      else
			{
			  if ((token[0] >= '0') && (token[0] <= '9'))
			    {
			      argument->td->is_array = array_dimension_index + 1;
			      argument->td->array_dimensions[array_dimension_index] = atoi(token);
			    }
			  else
			    {
			      cerr << "Error: invalid array dimension: " << token << endl;
			    }
			}
		      array_dimension_index++;
		    }
		  else if (!strcmp(token, "="))
		    {
		      // argument is optional.
		      argument->is_optional = 1;

		      // skip to the next comma.
		      // Keep in mind that initializers for char. strings
		      // may contain commas. (SbTime::formatDate)

		      line->tokens->next(placeholder);
		      token = (*(line->tokens))(placeholder);

		      if (!strcmp(token, "\"") &&
			  !strcmp(argument->td->type, "char"))
			{
			  // beginning of initializer for char string.
			  // skip to the end of it.

			  do
			    {
			      line->tokens->next(placeholder);
			      token = (*(line->tokens))(placeholder);
			      
			      if (!strcmp(token, "\\")) // escaping a character
				{
				  line->tokens->next(placeholder);
				  line->tokens->next(placeholder);
				  token = (*(line->tokens))(placeholder);
				}
			    }
			  while (strcmp(token, "\""));
			}
		    }
		      
		  line->tokens->next(placeholder);
		  token = (*(line->tokens))(placeholder);
		}
	      
	      // check to see whether this argument is actually "void",
	      // indicating no argument. Right now we're not checking
	      // whether this is the only argument in the argument list.

	      if ((!strcmp(argument->td->type, "void")) &&
		  (argument->td->ref == OBJECT))
		delete argument;
	      else
		method->args->append(argument);
	      
	      if (strcmp(token, ")") == 0)
		arglistdone = 1;
	    }
	}
      if (method->is_friend)
	{
	  if (strcmp(method->name, "operator"))  // i.e. not a friend operator
	    {
	      cerr << "Ignoring friend function \"" << method->name << "\" in class \"" <<
		name << "\"" << endl;
	      delete method;
	      return;
	    }
	}
      appendToMethods(method);
//      methods->append(method);
    }
}

char *
Class::conditionalUnscopeName(char *a_name)
{
  if (a_name == NULL)
    return NULL;

  if (parent_header->parent_directory->isScopedDataType(a_name))
    {
      int counter, flag = 0;
      for (counter = 0; counter < strlen(a_name); counter++)
	{
	  if ((flag == 1) && (a_name[counter] == ':'))
	    {
	      if (strncmp(a_name, name, counter - 1) == 0)
		{
		  char *newname = new char[strlen(a_name) - counter];
		  strcpy(newname, (a_name + counter + 1));
		  if (!strcmp(newname, name)) // constructor
		    {
		      delete[] newname;
		      return NULL;
		    }
		  return newname;
		}
	    }
	  else
	    {
	      if (a_name[counter] == ':')
		flag = 1;
	      else
		flag = 0;
	    }
	}
    }

  char *newname = new char[strlen(a_name) + 1];
  strcpy(newname, a_name);
  return newname;
}

void
Class::appendToMethods(Method *method)
{
  if (!parent_header->parent_directory->methodInIgnoresList(name, method->name, method->td->type))
    methods->append(method);
}

void
Class::print()
{
  Pix placeholder;
  Variable *vptr;
  Method *mptr;
  int counter;

  cout << "class " << name;
  if (superclasses->length() > 0)
    {
      char *sptr;
      Pix splc;
      cout << " : ";
      splc = superclasses->first();
      while (splc)
	{
	  cout << "public ";
	  sptr = (*superclasses)(splc);
	  cout << sptr;
	  superclasses->next(splc);
	  if (splc)
	    cout << ", ";
	}
    }
  cout << endl;
  cout << "Variables: " << endl;

  placeholder = variables->first();

  while (placeholder)
    {
      vptr = (*variables)(placeholder);
      cout << vptr->td->type << " ";
      if ((vptr->td->ref == POINTER) || (vptr->td->ref == REFERENCE_TO_POINTER))
	for (counter = 0; counter < vptr->td->depth; counter++)
	  cout << "*";
      if ((vptr->td->ref == REFERENCE) || (vptr->td->ref == REFERENCE_TO_POINTER))
	cout << "&";
      cout << vptr->name << ";" << endl;
      variables->next(placeholder);
    }

  cout << "Methods:" << endl;
 
  placeholder = methods->first();
  
  while (placeholder)
    {
      mptr = (*methods)(placeholder);
      mptr->print();
      methods->next(placeholder);
    }
  cout << "---" << endl;
}

int
Class::hasConstructor()
{
  Pix placeholder;

  placeholder = methods->first();
  while (placeholder)
    {
      Method *method = (*methods)(placeholder);
      char *methodname = method->name;
      if (!methodname)
	{
	  return 1;
	}
      methods->next(placeholder);
    }
  return 0;
}  

int
Class::hasDestructor()
{
  Pix p1;

  p1 = methods->first();
  while (p1)
    {
      Method *method = (*methods)(p1);
      if (strcmp(method->td->type, "~") == 0)
	return 1;
      methods->next(p1);
    }
  return 0;
}

void
Class::outputHeaderFile(FILE *fp)
{
  char *upperstring;
  upperstring = stringToUppercase(name);
  fprintf(fp, "#ifndef _SCM_%s_\n", upperstring);
  fprintf(fp, "#define _SCM_%s_\n", upperstring);
  fprintf(fp, "\n");
  fprintf(fp, "#include \"%s.h\"\n", parent_header->parent_directory->package_name);
  fprintf(fp, "\n");
  fprintf(fp, "extern long tc32_%s;\n", name);
  fprintf(fp, "extern ClassNode *SCM_%s_classnode;\n", name);

  fprintf(fp, "// get the class definition from our header file\n");
  SLList<char *> my_filename;
  my_filename.append(parent_header->filename);
  parent_header->parent_directory->outputFormattedIncludes(my_filename, fp);
//  fprintf(fp, "class %s;\n", name);


  fprintf(fp, "\n");
  fprintf(fp, "#define SCM2%s(x)  ((%s *) SCM2PTR(CDR(x)))\n", upperstring, name);
  fprintf(fp, "#define %sP(x)  (CELLP(x) ? (classController->searchUpForClass(SCM_%s_classnode,TYP32(x))) : 0)\n",
	  upperstring, name);
  fprintf(fp, "// Conversion functions from %s to SCM\n", name);
  fprintf(fp, "SCM SCM_%s(const %s *);\n", name, name);
  if (!exists_as_pointer_only)
    fprintf(fp, "SCM SCM_%s(const %s &);\n", name, name);
  fprintf(fp, "\n");
  fprintf(fp, "// Cast procedure\n");
  fprintf(fp, "SCM SCM_%s_Cast(SCM x ...);\n\n", name);
  fprintf(fp, "// Predicate procedure\n");
  fprintf(fp, "SCM SCM_%s_Predicate(SCM arglist ...);\n\n", name);
  fprintf(fp, "// Required functions for data type\n");
  fprintf(fp, "static int SCM_%s_Print(SCM, SCM, int);\n", name);
  fprintf(fp, "SCM SCM_%s_EqualP(SCM, SCM);\n", name);
  fprintf(fp, "SCM SCM_%s_Dispatch(char *method_name, SCM arglist);\n", name);
  if (hasDestructor())
    fprintf(fp, "sizet SCM_%s_Free(CELLPTR);\n", name);
  fprintf(fp, "\n");

  if (!strcmp(name, "void"))   /* special case -- needed in order to
				  give Scheme a notion of null pointers */
    {
      fprintf(fp, "// Special case: get a void null pointer in SCM\n");
      fprintf(fp, "SCM SCM_%s_NullPtr(SCM arglist ...);\n\n", name);
    }

  fprintf(fp, "// initializer function for registering this class with SCM\n");
  fprintf(fp, "void init_SCM_%s();\n\n", name);

  // output prototypes for methods and variable accesses
  outputPrototypes(fp);

  fprintf(fp, "\n#endif\n");
}

void
Class::outputPrototypes(FILE *fp)
{
  outputPrototypesForClassVariables(fp);
  outputPrototypesForMethods(fp);
}

void
Class::outputPrototypesForClassVariables(FILE *fp)
{
  // build up a list of all variables to give access to.

  SLList<char *> varnames;
  Pix placeholder;
  
  placeholder = variables->first();
  while (placeholder)
    {
      char *var_in = ((*variables)(placeholder))->name;
      char *lower_varname = stringToLowercase(var_in);
      
      conditionalAddStringToSLList(lower_varname, varnames);
      variables->next(placeholder);
    }
	
  // ok, now we have a list of the variables' names as they will appear
  // to the Scheme interface (i.e. in lowercase).
  // output prototypes for them. (This is easy)

  placeholder = varnames.first();
  while (placeholder)
    {
      char *varname = varnames(placeholder);
      fprintf(fp, "SCM SCM_%s_%s(SCM arglist ...);\n", name, varname);
      varnames.next(placeholder);
    }
}

void
Class::outputPrototypesForMethods(FILE *fp)
{
  int has_constructor = 0;

  // build up a list of all methods to give access to.

  SLList<char *> methodnames;
  Pix placeholder;
  
  placeholder = methods->first();
  while (placeholder)
    {
      Method *method = (*methods)(placeholder);
      char *methodname = method->name;
      if (!methodname)
	{
	  has_constructor = 1;
	}
      else
	{
	  if (strcmp(method->td->type, "~"))
	    {
	      char *lower_methodname;
	      if (method->operator_num)
		{
		  lower_methodname = new char[strlen("operator") + 4];
		  sprintf(lower_methodname, "operator%d", method->operator_num);
		}
	      else
		lower_methodname = stringToLowercase(methodname);
	      conditionalAddStringToSLList(lower_methodname, methodnames);
	    }
	}
      methods->next(placeholder);
    }
	
  // ok, now we have a list of the methods' names as they will appear
  // to the Scheme interface (i.e. in lowercase).
  // output prototypes for them. (This is easy)

  if (hasConstructor())
    {
      fprintf(fp, "SCM SCM_%s_constructor(SCM arglist ...);\n", name);
    }

  placeholder = methodnames.first();
  while (placeholder)
    {
      char *methodname = methodnames(placeholder);
      fprintf(fp, "SCM SCM_%s_%s(SCM arglist ...);\n", name, methodname);
      methodnames.next(placeholder);
    }
}  

void
Class::outputSchemeInterface(FILE *fp)
{
  convertClassVariablesToMethods();
  resolveTypeDefs();
  resolvePointers();
  prependObjectPointerToArgumentLists();
  expandOptionalArgumentMethods();
//  print();
  outputStandardDeclarations(fp);
  outputStandardSCMFunctions(fp);
  if (hasConstructor())
    outputConstructor(fp);
  outputMethods(fp);
}

void
Class::outputStandardDeclarations(FILE *fp)
{
  fprintf(fp, "static char *parent_classes[] = {");
  // print out all parent classes so they can be referenced by name
  Pix p1;
  p1 = superclasses->first();
  while (p1)
    {
      char *parent_name = (*superclasses)(p1);
      fprintf(fp, "\"%s\", ", parent_name);
      superclasses->next(p1);
    }
  // output a null ptr for the last one
  fprintf(fp, "0};\n\n");

  // output the type code for this class
  fprintf(fp, "long tc32_%s;\n", name);

  // output the ClassNode for this class
  fprintf(fp, "ClassNode *SCM_%s_classnode;\n\n", name);

  // if we have both a constructor and destructor for this class,
  // output a garbage collection list
  if (hasConstructor() && hasDestructor())
    {
      fprintf(fp, "static HashTable SCM_%s_gc_table;\n", name);
    }
  
  fprintf(fp, "extern int verbose;\n\n");

  // output the smob functions for this class
  fprintf(fp, "static smobfuns SCM_%s_smob = {mark0, ", name);
  
  if (hasConstructor() && hasDestructor())
    fprintf(fp, "SCM_%s_Free, ", name);
  else
    fprintf(fp, "free0, ");
  fprintf(fp, "SCM_%s_Print, SCM_%s_EqualP};\n\n", name, name);

  // output the static list of sorted iprocs for use in dispatch
  outputSortedIprocsAndInitializationProcedure(fp);
}
  
/* Outputs a list of iprocs sorted by their calling name from Scheme.
   This is later used in the dispatch procedure. */

void
Class::outputSortedIprocsAndInitializationProcedure(FILE *fp)
{
  SLList<char *> function_names;          // list of all member functions (and class variables)
  SLList<char *> scheme_function_names;   // names for these functions from within Scheme
  
  Method *m1;
  Pix p1;
  char **sorted_function_names = NULL;
  char **sorted_scheme_function_names = NULL;

  // we're going to build up a sorted list of names of
  // non-static member functions; i.e. those which can
  // be called using the '->' operator in Scheme.

  p1 = methods->first();
  while(p1)
    {
      m1 = (*methods)(p1);
      if ((m1->name != NULL) && (strcmp(m1->td->type, "~")))  // i.e. not a constructor or destructor (actual member function)
	{
	  if (!m1->is_static)
	    {
	      char *tempname = new char[256];
	      if (m1->operator_num)
		sprintf(tempname, "SCM_%s_operator%d", name, m1->operator_num);
	      else
		{
		  char *lowername = stringToLowercase(m1->name);
		  sprintf(tempname, "SCM_%s_%s", name, lowername);
		  delete[] lowername;
		}
	      if (conditionalAddStringToSLList(tempname, function_names) < 0)
		delete[] tempname;

	      char *tempschemename = new char[256];
	      if (m1->operator_num)
		sprintf(tempschemename, "operator%s", SchemeOperatorList[m1->operator_num - 1]);
	      else
		{
		  char *lowername = stringToLowercase(m1->name);
		  sprintf(tempschemename, "%s", lowername);
		  delete[] lowername;
		}
	      if (conditionalAddStringToSLList(tempschemename, scheme_function_names) < 0)
		delete[] tempschemename;
	    }
	}
      methods->next(p1);
    }

  if (function_names.length() > 0)
    {
      if (function_names.length() != scheme_function_names.length())
	{
	  cerr << "Class::outputSortedIprocsAndInitializationProcedure: class \"" << name << "\" contains methods/class variables which " << 
	    endl << "are the same except for capitalization differences (not allowed)" << endl;
	  return;
	}

      // need to convert the SLList into a char array.
      // then quicksort that array.
      sorted_function_names = SLListToCharArray(&function_names);
      sorted_scheme_function_names = SLListToCharArray(&scheme_function_names);

      quicksortIprocs(sorted_scheme_function_names, sorted_function_names, 0, function_names.length() - 1);
    }
  
  // output the dispatch iprocs (which are a subset of the real iprocs)

  fprintf(fp, "static iproc dispatch_iprocs[] = {\n");

  for (int c1 = 0; c1 < function_names.length(); c1++)
    {
      fprintf(fp, "{\"%s\", %s},\n", sorted_scheme_function_names[c1], sorted_function_names[c1]);
    }

  fprintf(fp, "{0, 0}};\n");
  fprintf(fp, "static int SCM_%s_iprocmin = 0;\n", name);
  fprintf(fp, "static int SCM_%s_iprocmax = %d;\n\n", name, function_names.length() - 1);

  // append all static methods to the end of the iproc list

  p1 = methods->first();
  while(p1)
    {
      m1 = (*methods)(p1);
      if ((m1->name != NULL) && (strcmp(m1->td->type, "~")))  // i.e. not a constructor or destructor (actual member function)
	{
	  if (m1->is_static)
	    {
	      char *tempname = new char[256];
	      if (m1->operator_num)
		sprintf(tempname, "SCM_%s_operator%d", name, m1->operator_num);
	      else
		{
		  char *lowername = stringToLowercase(m1->name);
		  sprintf(tempname, "SCM_%s_%s", name, lowername);
		  delete[] lowername;
		}
	      if (conditionalAddStringToSLList(tempname, function_names) < 0)
		delete[] tempname;

	      char *tempschemename = new char[256];
	      if (m1->operator_num)
		sprintf(tempschemename, "operator%s", SchemeOperatorList[m1->operator_num - 1]);
	      else
		{
		  char *lowername = stringToLowercase(m1->name);
		  sprintf(tempschemename, "%s", lowername);
		  delete[] lowername;
		}
	      if (conditionalAddStringToSLList(tempschemename, scheme_function_names) < 0)
		delete[] tempschemename;
	    }
	}
      methods->next(p1);
    }

  // output the list iprocs (direct references to all known methods)

  if (function_names.length() > 0)
    {
      if (function_names.length() != scheme_function_names.length())
	{
	  cerr << "Class::outputSortedIprocsAndInitializationProcedure <2>: class \"" << name << "\" contains methods/class variables which " << 
	    endl << "are the same except for capitalization differences (not allowed)" << endl;
	  return;
	}
    }

  fprintf(fp, "static iproc list_subrs[] = {\n");
  char *lowername = stringToLowercase(name);
  if (hasConstructor())
    {
      fprintf(fp, "{\"new-%s\", SCM_%s_constructor},\n", lowername, name);
    }

  fprintf(fp, "{\"%s-cast\", SCM_%s_Cast},\n", lowername, name);
  fprintf(fp, "{\"%s?\", SCM_%s_Predicate},\n", lowername, name);

  // special case for giving Scheme a notion of a null pointer
  if (!strcmp(name, "void"))
    fprintf(fp, "{\"%s-null\", SCM_%s_NullPtr},\n", lowername, name);

  Pix p2;
  char *t1, *t2;

  p1 = scheme_function_names.first();
  p2 = function_names.first();
  while (p1)
    {
      t1 = scheme_function_names(p1);
      t2 = function_names(p2);
      fprintf(fp, "{\"%s::%s\", %s},\n", lowername, t1, t2);
      scheme_function_names.next(p1);
      function_names.next(p2);
    }

  fprintf(fp, "{0, 0}};\n\n");
  
  // output the initialization function
  fprintf(fp, "void\ninit_SCM_%s()\n", name);
  fprintf(fp, "{\n");
  fprintf(fp, "tc32_%s = newsmob(&SCM_%s_smob);\n", name, name);
  if (hasConstructor() && hasDestructor())
    fprintf(fp, "SCM_%s_gc_table = HashTableCreate(1, NULL);\n", name);
  fprintf(fp, "if ((SCM_%s_classnode = classController->addClassToHierarchy(parent_classes,\n", name);
  fprintf(fp, "tc32_%s,\n", name);
  fprintf(fp, "\"%s\")) == NULL)\n", name);
  fprintf(fp, "{\n");
  fprintf(fp, "fprintf(stderr, \"FATAL ERROR while adding class %s to the class hierarchy\\n\");\n", name);
  fprintf(fp, "exit(1);\n");
  fprintf(fp, "}\n");
  fprintf(fp, "SCM_%s_classnode->dispatch_func = SCM_%s_Dispatch;\n", name, name);
  fprintf(fp, "init_iprocs(list_subrs, tc7_lsubr);\n");
  fprintf(fp, "}\n\n");
}

void
Class::quicksortIprocs(char **a,
		       char **b,
		       int l, int r)
{
  int i, j;
  char *t, *v;
  if (r > l)
    {
      v = a[r]; i = l-1; j = r;
      for (;;)
	{
	  while ((++i <= r) && (strcmp(a[i],v) < 0));
//	    {
//	      cerr << "i comparison of \"" << a[i] << "\" to \"" << v << "\": " << strcmp(a[i], v) << endl;
//	    }
	  while ((--j >= 0) && (strcmp(a[j],v) > 0));
//	    {
//	      cerr << "j comparison of \"" << a[j] << "\" to \"" << v << "\": " << strcmp(a[j], v) << endl;
//	    }
	  if (i >= j) break;
	  t = a[i]; a[i] = a[j]; a[j] = t;
	  t = b[i]; b[i] = b[j]; b[j] = t;
	}
      t = a[i]; a[i] = a[r]; a[r] = t;
      t = b[i]; b[i] = b[r]; b[r] = t;
      quicksortIprocs(a, b, l, i-1);
      quicksortIprocs(a, b, i+1, r);
    }
}

void
Class::outputStandardSCMFunctions(FILE *fp)
{
  if (hasConstructor() && hasDestructor())
    outputFreeProcedure(fp);
  outputPrintProcedure(fp);
  outputEqualPProcedure(fp);
  outputCastProcedure(fp);
  outputPredicateProcedure(fp);
  outputConversionProcedures(fp);
  // special case to give Scheme
  // a notion of null pointers
  if (!strcmp(name, "void"))
    outputNullProcedure(fp);
  outputDispatchProcedure(fp);
}

void
Class::outputFreeProcedure(FILE *fp)
{
  char *upperstring = stringToUppercase(name);

  fprintf(fp, "sizet\nSCM_%s_Free(CELLPTR gc_cell)\n", name);
  fprintf(fp, "{\n");
  fprintf(fp, "%s *freeptr = SCM2%s(gc_cell);\n", name, upperstring);
  fprintf(fp, "HashEntry *entry;\n\n");
  fprintf(fp, "// This assumes a 32 bit architecture\n");
  fprintf(fp, "int key = (int) freeptr;\n");
  fprintf(fp, "\n");
  fprintf(fp, "if ((entry = HashTableFindEntry(SCM_%s_gc_table, (HashKey) key)) != NULL)\n", name);
  fprintf(fp, "{\n");
  fprintf(fp, "freeptr->~%s();\n", name);
  fprintf(fp, "must_free((char *)freeptr);\n");
  fprintf(fp, "HashTableRemoveEntry(SCM_%s_gc_table, entry);\n", name);
  fprintf(fp, "if (verbose > 3)\n");
  fprintf(fp, "cerr << \"; GC'd %s \" << (void *) freeptr << endl;\n", name);
  fprintf(fp, "return (sizet) sizeof(%s);\n", name);
  fprintf(fp, "}\n");
  fprintf(fp, "return (sizet) 0;\n");
  fprintf(fp, "}\n\n");

  delete[] upperstring;
}
  
void
Class::outputPrintProcedure(FILE *fp)
{
  char *upperstring = stringToUppercase(name);
  char *lowerstring = stringToLowercase(name);
  fprintf(fp, "static int\nSCM_%s_Print(SCM exp,\nSCM port,\nint)\n", name);
  fprintf(fp, "{\n");
  fprintf(fp, "char tmpstr[256];\n\n");
  fprintf(fp, "%s *a_%s = SCM2%s(exp);\n", name, lowerstring, upperstring);
  fprintf(fp, "sprintf(tmpstr, \"#<%s: %%p>\", a_%s);\n", name, lowerstring);
  fprintf(fp, "lputs(tmpstr, port);\n");
  fprintf(fp, "return !0;\n");
  fprintf(fp, "}\n\n");
  delete[] upperstring;
  delete[] lowerstring;
}

void
Class::outputEqualPProcedure(FILE *fp)
{
  char *upperstring = stringToUppercase(name);
  fprintf(fp, "SCM\nSCM_%s_EqualP(SCM x, SCM y)\n", name);
  fprintf(fp, "{\n");
  fprintf(fp, "if (%sP(x) && %sP(y))\n", upperstring, upperstring);
  fprintf(fp, "if (CDR(x) == CDR(y))\n");
  fprintf(fp, "return BOOL_T;\n");
  fprintf(fp, "return BOOL_F;\n");
  fprintf(fp, "}\n\n");
  delete[] upperstring;
}

void
Class::outputCastProcedure(FILE *fp)
{
  char *upperstring = stringToUppercase(name);

  fprintf(fp, "SCM\nSCM_%s_Cast(SCM x ...)\n{\n", name);
  fprintf(fp, "int i = ilength(x);\n");
  fprintf(fp, "ASSERT((i == 1), x, WNA, \"SCM_%s_Cast\");\n\n", name);

  fprintf(fp, "ASSERT(CELLP(CAR(x)), CAR(x), ARG1, \"SCM_%s_Cast\");\n", name);
  fprintf(fp, "return SCM_%s(SCM2%s(CAR(x)));\n", name, upperstring);

//  fprintf(fp, "NEWCELL(retval);\n\n");
//  fprintf(fp, "CAR(retval) = tc32_%s;\n", name);
//  fprintf(fp, "CDR(retval) = CDR(CAR(x));\n");
//  fprintf(fp, "return(retval);\n");
  fprintf(fp, "}\n\n");
}

void
Class::outputPredicateProcedure(FILE *fp)
{
  char *upperstring = stringToUppercase(name);
  fprintf(fp, "SCM\nSCM_%s_Predicate(SCM arglist ...)\n", name);
  fprintf(fp, "{\n");
  fprintf(fp, "ASSERT(ilength(arglist)==1, arglist, WNA, \"SCM_%s_Predicate\");\n", name);
  fprintf(fp, "return (%sP(CAR(arglist)) ? BOOL_T : BOOL_F);\n", upperstring);
  fprintf(fp, "}\n\n");
  delete[] upperstring;
}

void
Class::outputConversionProcedures(FILE *fp)
{
  char *lowerstring = stringToLowercase(name);
  char *upperstring = stringToUppercase(name);

  if (!exists_as_pointer_only)
    {
      fprintf(fp, "SCM\nSCM_%s(const %s &a_%s)\n", name, name, lowerstring);
      fprintf(fp, "{\n");
      fprintf(fp, "SCM temp;\n");
      fprintf(fp, "HashEntry *entry;\n");
      fprintf(fp, "int found;\n");
      fprintf(fp, "\n");
      fprintf(fp, "// These assume a 32-bit architecture. Sorry.\n");
      fprintf(fp, "int key[2];\n");
      fprintf(fp, "key[0] = (int) tc32_%s;\n", name);
      fprintf(fp, "key[1] = (int) &a_%s;\n", lowerstring);
      fprintf(fp, "entry = HashTableAddEntry(hashTable, (HashKey) key, &found);\n\n");
      fprintf(fp, "if (found)\n");
      fprintf(fp, "{\n");
      fprintf(fp, "temp = obunhash((SCM) entry->value);\n");
      fprintf(fp, "if (%sP(temp) && (CDR(temp) == PTR2SCM(&a_%s)))\n", upperstring, lowerstring);
      fprintf(fp, "{\n");
      fprintf(fp, "if (verbose > 4)\n");
      fprintf(fp, "cerr << \"; Hash found %s \" << (void *) &a_%s << endl;\n", name, lowerstring);
      fprintf(fp, "return temp;\n");
      fprintf(fp, "}\n");
      fprintf(fp, "}\n\n");
      fprintf(fp, "// Couldn't find this pointer in the hash table, so allocate space for it.\n\n");
      fprintf(fp, "NEWCELL(temp);\n");
      fprintf(fp, "\n");
      fprintf(fp, "CAR(temp) = tc32_%s;\n", name);
      fprintf(fp, "CDR(temp) = PTR2SCM(&a_%s);\n", lowerstring);
      fprintf(fp, "entry->value = (void *) obhash(temp);\n\n");
      fprintf(fp, "return temp;\n");
      fprintf(fp, "}\n\n");
    }

  fprintf(fp, "SCM\nSCM_%s(const %s *a_%s)\n", name, name, lowerstring);
  fprintf(fp, "{\n");
  fprintf(fp, "SCM temp;\n");
  fprintf(fp, "HashEntry *entry;\n");
  fprintf(fp, "int found;\n");
  fprintf(fp, "\n");
  fprintf(fp, "// These assume a 32-bit architecture. Sorry.\n");
  fprintf(fp, "int key[2];\n");
  fprintf(fp, "key[0] = (int) tc32_%s;\n", name);
  fprintf(fp, "key[1] = (int) a_%s;\n", lowerstring);
  fprintf(fp, "entry = HashTableAddEntry(hashTable, (HashKey) key, &found);\n\n");
  fprintf(fp, "if (found)\n");
  fprintf(fp, "{\n");
  fprintf(fp, "temp = obunhash((SCM) entry->value);\n");
  fprintf(fp, "if (%sP(temp) && (CDR(temp) == PTR2SCM(a_%s)))\n", upperstring, lowerstring);
  fprintf(fp, "{\n");
  fprintf(fp, "if (verbose > 4)\n");
  fprintf(fp, "cerr << \"; Hash found %s \" << (void *) a_%s << endl;\n", name, lowerstring);
  fprintf(fp, "return temp;\n");
  fprintf(fp, "}\n");
  fprintf(fp, "}\n\n");
  fprintf(fp, "// Couldn't find this pointer in the hash table, so allocate space for it.\n\n");
  fprintf(fp, "NEWCELL(temp);\n");
  fprintf(fp, "\n");
  fprintf(fp, "CAR(temp) = tc32_%s;\n", name);
  fprintf(fp, "CDR(temp) = PTR2SCM(a_%s);\n", lowerstring);
  fprintf(fp, "entry->value = (void *) obhash(temp);\n\n");
  fprintf(fp, "return temp;\n");
  fprintf(fp, "}\n\n");

  delete[] lowerstring;
  delete[] upperstring;
}

void
Class::outputNullProcedure(FILE *fp)
{
  fprintf(fp, "SCM\n");
  fprintf(fp, "SCM_%s_NullPtr(SCM arglist ...)\n", name);
  fprintf(fp, "{\n");
  fprintf(fp, "  ASSERT(ilength(arglist)==0, arglist, WNA, \"SCM_%s_NullPtr\");\n", name);
  fprintf(fp, "  SCM temp;\n");
  fprintf(fp, "  NEWCELL(temp);\n");
  fprintf(fp, "  CAR(temp) = tc32_%s;\n", name);
  fprintf(fp, "  CDR(temp) = PTR2SCM((%s *)NULL);\n", name);
  fprintf(fp, "  return temp;\n");
  fprintf(fp, "}\n\n");
}
  
void
Class::outputDispatchProcedure(FILE *fp)
{
  fprintf(fp, "SCM\n");
  fprintf(fp, "SCM_%s_Dispatch(char *method_name, SCM arglist)\n", name);
  fprintf(fp, "{\n");
  fprintf(fp, "Pix placeholder;\n");
  fprintf(fp, "ClassNode *parent_node;\n");
  fprintf(fp, "SCM (*function)(SCM, ...) = iprocBinarySearch(dispatch_iprocs, \n");
  fprintf(fp, "method_name,\n");
  fprintf(fp, "SCM_%s_iprocmin,\n", name);
  fprintf(fp, "SCM_%s_iprocmax);\n", name);
  fprintf(fp, "SCM retval;\n\n");
  fprintf(fp, "if (function)\n");
  fprintf(fp, "{\n");
  fprintf(fp, "return (*function)(arglist);\n");
  fprintf(fp, "}\n");
  fprintf(fp, "else\n");
  fprintf(fp, "{\n");
  fprintf(fp, "// do traversal of tree.\n");
  fprintf(fp, "// doesn't matter which order we do it in,\n");
  fprintf(fp, "// since if there's any ambiguity about the method to call\n");
  fprintf(fp, "// (i.e. multiple inheritance) then it's the user's responsibility\n");
  fprintf(fp, "// to take care of it.\n");
  fprintf(fp, "placeholder = SCM_%s_classnode->parents->first();\n", name);
  fprintf(fp, "while (placeholder)\n");
  fprintf(fp, "{\n");
  fprintf(fp, "parent_node = (*(SCM_%s_classnode->parents))(placeholder);\n", name);
  fprintf(fp, "retval = (*(parent_node->dispatch_func))(method_name, arglist);\n");
  fprintf(fp, "if (retval)\n");
  fprintf(fp, "return retval;\n");
  fprintf(fp, "SCM_%s_classnode->parents->next(placeholder);\n", name);
  fprintf(fp, "}\n");
  fprintf(fp, "return NULL;\n");
  fprintf(fp, "}\n");
  fprintf(fp, "}\n\n");
}

void
Class::resolveTypeDefs()
{
  // merges typedefs with current type declarations.

  // do methods first

  Pix p1;
  Method *m1;
  TypeDef *td;
  p1 = methods->first();
  while (p1)
    {
      m1 = (*methods)(p1);
      
      // first check the method's return type
      if (td = parent_header->parent_directory->typeInTypesList(m1->td->type))
	{
	  mergeTypeDescWithTypeDef(m1->td, td);
	}

      // now check all arguments to the method

      Pix p2;
      Argument *a2;
      p2 = m1->args->first();
      while (p2)
	{
	  a2 = (*(m1->args))(p2);
	  
	  if (td = parent_header->parent_directory->typeInTypesList(a2->td->type))
	    {
	      mergeTypeDescWithTypeDef(a2->td, td);
	    }
	  m1->args->next(p2);
	}
      methods->next(p1);
    }

  // now variables

  Variable *v1;
  p1 = variables->first();
  while (p1)
    {
      v1 = (*variables)(p1);
      if (td = parent_header->parent_directory->typeInTypesList(v1->td->type))
	{
	  mergeTypeDescWithTypeDef(v1->td, td);
	}
      variables->next(p1);
    }
}
	  
void
Class::mergeTypeDescWithTypeDef(TypeDesc *tds, TypeDef *tdf)
{
  if (tdf->resolved_type->is_enum)
    {
      tds->enumerated_type = tds->type;
      tds->is_enum = 1;
    }
  else
    {
      if (tds->type)
	delete[] tds->type;
    }

  tds->type = new char[strlen(tdf->resolved_type->type) + 1];
  strcpy(tds->type, tdf->resolved_type->type);
  if (tdf->resolved_type->is_array)
    {
      if (tds->is_array + tdf->resolved_type->is_array > 5)
	{
	  cerr << "Class::mergeTypeDescWithTypeDef: too many dimensions in array" << endl;
	}
      else
	{
	  for (int c1 = 0; c1 < tdf->resolved_type->is_array; c1++)
	    {
	      tds->array_dimensions[tds->is_array + c1] = tdf->resolved_type->array_dimensions[c1];
	    }
	  tds->is_array += tdf->resolved_type->is_array;
	}
    }
  tds->is_short = tdf->resolved_type->is_short;
  tds->is_long = tdf->resolved_type->is_long;
}

void
Class::expandOptionalArgumentMethods()
{
  // finds all methods with optional arguments and adds
  // copies of them to the argument list, successively removing
  // the last argument in each method's argument list until there are
  // no optional arguments left. (this is to allow the differentiation
  // algorithm later to handle optional arguments.)

  Pix p1;
  Method *m1;

  p1 = methods->first();
  while (p1)
    {
      m1 = (*methods)(p1);
      
      Argument *a1;

      if (m1->args->length() > 0)
	{
	  a1 = m1->args->rear();
	  if (a1->is_optional)
	    {
	      expandMethodWithOptionalArgs(m1);
	    }
	}
      methods->next(p1);
    }
}

void
Class::expandMethodWithOptionalArgs(Method *m1)
{
  Method *m2 = new Method;
  m2->name = m1->name;
  m2->td = m1->td;
  m2->is_static = m1->is_static;
  m2->is_constant = m1->is_constant;
  m2->is_friend = m1->is_friend;
  m2->represents_class_variable = m1->represents_class_variable;
  m2->operator_num = m1->operator_num;

  // copy all but last argument of m1 into m2's arg list

  Pix p1;
  Argument *a1;

  p1 = m1->args->first();
  for (int c1 = 0; c1 < m1->args->length() - 1; c1++)
    {
      a1 = (*(m1->args))(p1);
      m2->args->append(a1);
      m1->args->next(p1);
    }
  
  // check to see if m2's last argument is optional
  if (m2->args->length() > 0)
    {
      a1 = m2->args->rear();
      if (a1->is_optional)
	{
	  expandMethodWithOptionalArgs(m2);

	  // we have to now say that m2's last argument is non-optional
	  // to prevent an infinite loop
	  a1->is_optional = 0;
	}
    }

  // add m2 to the method list
  methods->append(m2);
}
void
Class::resolvePointers()
{
  // resolves pointer ambiguities in function return values and argument lists.

  Pix p1;
  Method *m1;
  int userchoice;

  p1 = methods->first();
  while (p1)
    {
      m1 = (*methods)(p1);
      if ((m1->td->ref == POINTER) || (m1->td->ref == REFERENCE_TO_POINTER))
	{
	  if (m1->td->depth == 1)
	    {
	      // ambiguity noted.
	      // let's see if the user's defined this in the hints

	      if (!(userchoice = parent_header->parent_directory->typeDescInHintsList(m1->td)))
		{
		askuser:
		  cout << "In class " << name << ":" << endl;
		  m1->print();
		  cout << "Ambiguity: method returns a pointer. Please choose one option:" << endl;
		  cout << "1. Pointer to object" << endl;
		  cout << "2. Fixed length array" << endl;
		  if (!strcmp(m1->td->type, "char"))
		    {
		      cout << "3. Character string" << endl;
		      userchoice = getUserChoice("Your choice: ", 1, 3);
		    }
		  else
		    {
		      userchoice = getUserChoice("Your choice: ", 1, 2);
		    }
		}
	      
	      if (userchoice == 1)
		m1->td->ref = POINTER_TO_OBJECT;
	      else if (userchoice == 2)
		{
		  cout << "Enter array length (between 1 and " << INT_MAX << ")" << endl;
		  userchoice = getUserChoice("Length: ", 1, INT_MAX);
		  m1->td->is_array = 1;
		  m1->td->array_dimensions[0] = userchoice;
		}
	      else if (userchoice == 3)
		{
		  m1->td->ref = STRING;
		}
	      else
		{
		  cerr << "Class::resolvePointers: bad user choice in class \"" << name << 
		    "\", method \"" << m1->name << "\"" << endl;
		  goto askuser;
		}
	    }
	  else
	    {
	      cout << "In class " << name << ":" << endl;
	      m1->print();
	      cout << "Method returns multiply referenced pointer (assumed to be fixed length array)." << endl;
	      cout << "Please enter array dimensions (between 1 and " << INT_MAX << "):" << endl;
	      for (int c1 = 1; c1 <= m1->td->depth; c1++)
		{
		  char buf[255];
		  sprintf(buf, "Dimension %d: ", c1);
		  userchoice = getUserChoice(buf, 1, INT_MAX);
		  m1->td->array_dimensions[c1 - 1] = userchoice;
		}
	      m1->td->is_array = m1->td->depth;
	    }
	}

      // now do arguments

      int argcounter = 0;
      Pix p2;
      Argument *a2;
      
      p2 = m1->args->first();
      while (p2)
	{
	  a2 = (*(m1->args))(p2);
	  argcounter++;

	  if ((a2->td->ref == POINTER) || (a2->td->ref == REFERENCE_TO_POINTER))
	    {
	      if (a2->td->depth == 1)
		{
		  // ambiguity noted.
		  // let's see if the user's defined this in the hints
		  
		  if (!(userchoice = parent_header->parent_directory->typeDescInHintsList(a2->td)))
		    {
		    askuser2:
		      cout << "In class " << name << ":" << endl;
		      m1->print();
		      cout << "Ambiguity: argument " << argcounter << " is a pointer. Please choose one option:" << endl;
		      cout << "1. Pointer to object" << endl;
		      cout << "2. Fixed length array" << endl;
		      if (!strcmp(a2->td->type, "char"))
			{
			  cout << "3. Character string" << endl;
			  userchoice = getUserChoice("Your choice: ", 1, 3);
			}
		      else
			{
			  userchoice = getUserChoice("Your choice: ", 1, 2);
			}
		    }

		  if (userchoice == 1)
		    a2->td->ref = POINTER_TO_OBJECT;
		  else if (userchoice == 2)
		    {
		      cout << "Enter array length (between 1 and " << INT_MAX << ")" << endl;
		      userchoice = getUserChoice("Length: ", 1, INT_MAX);
		      a2->td->is_array = 1;
		      a2->td->array_dimensions[0] = userchoice;
		    }
		  else if (userchoice == 3)
		    {
		      a2->td->ref = STRING;
		    }
		  else
		    {
		      cerr << "Class::resolvePointers: bad user choice in class \"" << name << 
			"\", method \"" << m1->name << "\", argument type " << a2->td->type << endl;
		      goto askuser2;
		    }
		}
	      else
		{
		  cout << "In class " << name << ":" << endl;
		  m1->print();
		  cout << "Argument " << argcounter << " is multiply referenced pointer (assumed to be fixed length array)." << endl;
		  cout << "Please enter array dimensions (between 1 and " << INT_MAX << "):" << endl;
		  char buf[255];
		  for (int c1 = 1; c1 <= a2->td->depth; c1++)
		    {
		      sprintf(buf, "Dimension %d: ", c1);
		      userchoice = getUserChoice(buf, 1, INT_MAX);
		      a2->td->array_dimensions[c1 - 1] = userchoice;
		    }
		  a2->td->is_array = a2->td->depth;
		}
	    }
	  m1->args->next(p2);
	}
      methods->next(p1);
    }
}
  

void
Class::prependObjectPointerToArgumentLists()
{
  // Prepends an object pointer to all non-static and non-constructor methods,
  // because Scheme's calling syntax requires that the object for which the
  // method is being called be the first argument. So we add this pointer here
  // to allow the generality of the functions which handle function overloading
  // below to deal with extracting this pointer.

  Pix p1;
  Method *m1;

  p1 = methods->first();
  while (p1)
    {
      m1 = (*methods)(p1);
      
      if (!((m1->is_static) || (m1->name == NULL) || (m1->is_friend)))
	{
	  Argument *arg = new Argument;
	  arg->td->type = new char[strlen(name) + 1];  // object of this class type
	  strcpy(arg->td->type, name);
	  arg->td->ref = POINTER_TO_OBJECT;
	  arg->td->depth = 1;
	  
	  m1->args->prepend(arg);
	}
      methods->next(p1);
    }
}

/* This function converts all class variables into what look to the compiler
   like member functions. This allows accessors for class variables to be outputted
   in the same way that member functions are. */

void
Class::convertClassVariablesToMethods()
{
  Pix p1;
  Variable *v1;

  p1 = variables->first();
  while (p1)
    {
      v1 = (*variables)(p1);
      Method *m1 = new Method;
      
      // method's return type becomes the type of the class variable.
      m1->td = v1->td;

      // if the variable is a static class variable, it becomes a static member function
      m1->is_static = v1->is_static;

      // if the variable is a constant class variable, return value of the member function is also const
      m1->is_constant = v1->is_constant;

      // method's name is the same as the variable's name
      m1->name = v1->name;

      // tell the compiler that this method actually represents a class variable
      // (i.e. don't try to call it as a function)
      m1->represents_class_variable = 1;

      // I think this will take care of trying to pass arrays out of a function
      if ((m1->td->ref == OBJECT) && (m1->td->is_array > 0))
	m1->td->ref = REFERENCE;

      // difference: there is one argument to this member function if it is not static,
      // i.e. the object from which you want to extract the class variable.
      // note that this is the implicit "this" pointer, and will be added automatically
      // by "prependObjectPointerToArgumentLists", above, later on.

      appendToMethods(m1);
//      methods->append(m1);

      variables->next(p1);
    }
}

void
Class::outputConstructor(FILE *fp)
{
  fprintf(fp, "#include <new.h>\n\n");

  SLList<Method *> *cms;  // constructor methods

  cms = getMethodsByName(NULL);  /* constructor has
				    no name; return
				    type is name of
				    class */
  if (cms->length() > 0)
    {
      int longest_arglist = getLongestArgListLength(cms);
      outputFunctionHeaderAndSwitch(fp, "Constructor", 0, longest_arglist);
    }
  else
    return;

  int counter = 0;
  while (cms->length() > 0)
    {
      SLList <Method *> *tms = getMethodsOfArgLength(cms, counter);

      // figure out whether we can tell the difference between these methods
      // and cut down the list if we can't

      verifyMethodsAreDistinguishable(tms);

      // now we have a list of methods which are distinguishable from Scheme's
      // point of view. output the case clause for this length of argument list.

      outputMethodCaseClause(fp, counter, tms);
      
      // increment the argument list length
      counter++;
      delete tms;
    }
  outputDefaultCaseClause(fp, "Constructor", 0);
  
  // close switch
  fprintf(fp, "}\n");
  // close function
  fprintf(fp, "}\n\n");
  
  delete cms;
}

void
Class::outputMethods(FILE *fp)
{
  SLList<Method *> *ms;  // methods
  Pix p1;
  Method *m1;

  p1 = methods->first();
  while (p1)
    {
      m1 = (*(methods))(p1);
      if (strcmp(m1->td->type, "~"))
	{
	  ms = getMethodsByName(m1->name, m1->operator_num);
	  
	  if (ms->length() > 0)
	    {
	      int longest_arglist = getLongestArgListLength(ms);
	      outputFunctionHeaderAndSwitch(fp, m1->name, m1->operator_num, longest_arglist);
	    }
	  else
	    {
	      delete ms;
	      return;
	    }
	  
	  int counter = 0;
	  while (ms->length() > 0)
	    {
	      SLList <Method *> *tms = getMethodsOfArgLength(ms, counter);
	      
	      // figure out whether we can tell the difference between these methods
	      // and cut down the list if we can't
	      
	      verifyMethodsAreDistinguishable(tms);
	      
	      // now we have a list of methods which are distinguishable from Scheme's
	      // point of view. output the case clause for this length of argument list.
	      
	      outputMethodCaseClause(fp, counter, tms);
	      
	      // increment the argument list length
	      counter++;
	      delete tms;
	    }
	  
	  outputDefaultCaseClause(fp, m1->name, m1->operator_num);
	  
	  // close switch
	  fprintf(fp, "}\n");
	  // close function
	  fprintf(fp, "}\n\n");

	  // clean up by deleting the SLList of methods
	  // that were all the methods of a given name
	  delete ms;
	}
      else
	{
	  methods->remove_front();  // remove destructor
	}
      p1 = methods->first();
    }
}  

SLList<Method *> *
Class::getMethodsByName(char *method_name, int operator_num)
{
  // this function destructively modifies the methods list and
  // returns all methods whose name matches the passed parameter.

  SLList <Method *> *tl = new SLList<Method *>; // temp list
  Method *front, *ptr = NULL;
  
 repeat:
  if (methods->length() > 0)
    {
      front = methods->remove_front();
      // check to see if this method matches the search criterion
      if (((method_name == NULL) && (front->name == NULL)) ||
	  ((method_name != NULL) && (front->name != NULL) && (!strcmp(front->name, method_name)) && (front->operator_num == operator_num)))
	{
	  // add this method to the temp list
	  tl->append(front);
	  // go back to removing the front of the list (since as
	  // yet we can't tell when we've gone through the entire list)
	  goto repeat;
	}
      else
	{
	  // append this method back on the end
	  methods->append(front);

	  // keep stepping through the methods list until we reach
	  // this pointer again

	  do
	    {
	      ptr = methods->remove_front();

	      if (((method_name == NULL) && (ptr->name == NULL)) ||
		  ((method_name != NULL) && (ptr->name != NULL) && (!strcmp(ptr->name, method_name)) && (ptr->operator_num == operator_num)))
		{
		  tl->append(ptr);
		}
	      else
		{
		  if (ptr != front)
		    {
		      methods->append(ptr);
		    }
		  else
		    {
		      // keep iterations in order (does this really matter?)
		      methods->prepend(ptr);
		    }
		}
	    }
	  while (ptr != front);
	}
    }
  return tl;
}

int
Class::getLongestArgListLength(SLList<Method *> *mlist)
{
  Method *ptr;
  int len = 0;
  Pix pl;

  pl = mlist->first();
  while (pl)
    {
      ptr = (*mlist)(pl);
      if (ptr->args->length() > len)
	len = ptr->args->length();
      mlist->next(pl);
    }
  return len;
}

void
Class::outputFunctionHeaderAndSwitch(FILE *fp, char *methodname, int operator_num, int maxargs)
{
  char *lowername = stringToLowercase(methodname);

  fprintf(fp, "SCM\n");
  if (operator_num)  // method is an operator
    fprintf(fp, "SCM_%s_operator%d(SCM arglist ...)\n", name, operator_num);
  else
    fprintf(fp, "SCM_%s_%s(SCM arglist ...)\n", name, lowername);
  fprintf(fp, "{\n");
  fprintf(fp, "int i = ilength(arglist);\n");
  if (maxargs > 0)
    {
      fprintf(fp, "SCM ");
      for (int counter = 1; counter <= maxargs; counter++)
	{
	  fprintf(fp, "arg%d", counter);
	  if (counter == maxargs)
	    fprintf(fp, ";\n");
	  else
	    fprintf(fp, ", ");
	}
    }
  fprintf(fp, "switch(i)\n");
  fprintf(fp, "{\n");
  delete[] lowername;
}
  
void
Class::outputDefaultCaseClause(FILE *fp, char *method_name, int operator_num)
{
  char *lowername = stringToLowercase(method_name);

  fprintf(fp, "default:\n");
  if (operator_num)
    fprintf(fp, "ASSERT(0, arglist, WNA, \"SCM_%s_%s%s\");\n", name, lowername, OperatorList[operator_num - 1]);
  else
    fprintf(fp, "ASSERT(0, arglist, WNA, \"SCM_%s_%s\");\n", name, lowername);

  delete[] lowername;
}

SLList<Method *> *
Class::getMethodsOfArgLength(SLList<Method *> *ml, int len)
{
  // destructively modifies the passed method list and
  // removes all methods of argument length equal to len.

  SLList <Method *> *tl = new SLList<Method *>; // temp list
  Method *front, *ptr = NULL;
  int result;
  
 repeat:
  result = ml->remove_front(front);
  if (result)
    {
      // check to see if this method matches the search criterion
      if (front->args->length() == len)
	{
	  // add this method to the temp list
	  tl->append(front);
	  // go back to removing the front of the list (since as
	  // yet we can't tell when we've gone through the entire list)
	  goto repeat;
	}
      else
	{
	  // append this method back on the end
	  ml->append(front);

	  // keep stepping through the methods list until we reach
	  // this pointer again

	  do
	    {
	      ptr = ml->remove_front();

	      if (ptr->args->length() == len)
		{
		  tl->append(ptr);
		}
	      else
		{
		  if (ptr != front)
		    {
		      ml->append(ptr);
		    }
		  else
		    {
		      // keep iterations in order (does this really matter?)
		      ml->prepend(ptr);
		    }
		}
	    }
	  while (ptr != front);
	}
    }
  return tl;
}

void
Class::verifyMethodsAreDistinguishable(SLList<Method *> *ml)
{
  // figures out whether (from Scheme's point of view) the methods
  // in the passed list are distinguishable.

  SLList<Method *> t1, t2, t3;  // temp lists
  Method *front, *ptr;
  int result;
  
 repeat:
  if (ml->length() > 0)
    {
      front = ml->remove_front();
      // we want to see which of the other methods in the passed list
      // seem indistinguishable to Scheme.
      
      t1.append(front);
      do
	{
	  result = ml->remove_front(ptr);
	  if (result)
	    {
	      if (methodsAreIndistinguishable(front, ptr))
		{
		  t1.append(ptr);
		}
	      else
		{
		  t2.append(ptr);
		}
	    }
	}
      while (result);

      // now t1 contains all the methods indistinguishable to "front".
      // if its length is larger than 1, force the user to choose one

      if (t1.length() > 1)
	{
	  forceUserToChooseOneMethod(&t1);
	}

      // now t1 contains exactly one method.
      // append it to t3
      
      if (t1.length() != 1)
	{
	  cerr << "Class::verifyMethodsAreDistinguishable: method list is unexpectedly too large!" << endl;
	}
      else
	{
	  t3.append(t1.remove_front());
	}

      // copy t2 back into ml

      while (t2.length() > 0)
	{
	  ml->append(t2.remove_front());
	}

      // do this again until we reach the end of ml
      goto repeat;
    }
  else
    {
      // copy t3 (the final list) back into ml
      while (t3.length() > 0)
	{
	  ml->append(t3.remove_front());
	}
    }
}

void
Class::outputMethodCaseClause(FILE *fp, int argslen, SLList<Method *> *ml)
{
  if (ml->length() > 0)
    {
      fprintf(fp, "case %d:\n", argslen);
      fprintf(fp, "{\n");
      if (argslen == 0)
	{
	  // there had better be only one method in the
	  // method list; i.e. one function with a given name
	  // with no arguments.

	  if (ml->length() > 1)
	    {
	      cerr << "Class::outputMethodCaseClause: more than one method with 0 arguments " <<
		"for class " << name << endl;
	      return;
	    }
	  
	  if (ml->length() < 1)
	    {
	      cerr << "Class::outputMethodCaseClause: no method with 0 arguments for class " <<
		name << endl;
	      return;
	    }
	  // output the function call for this method directly from here
	  Method *m1 = ml->remove_front();
	  outputFunctionCall(fp, m1);
	  outputReturnValue(fp, m1);
	}
      else
	{
	  Pix p1;
	  Method *m1;
	  p1 = ml->first();
	  m1 = (*ml)(p1);
	  outputArgListExtractions(fp, argslen);
	  outputBodyOfCaseClause(fp, ml, 1);
	}
      fprintf(fp, "}\n");
    }
}

void
Class::outputArgListExtractions(FILE *fp, int argslen)
{
  int c1, c2;

  for (c1 = 1; c1 <= argslen; c1++)
    {
      fprintf(fp, "arg%d = CAR(", c1);
      for (c2 = 1; c2 < c1; c2++)
	fprintf(fp, "CDR(");
      fprintf(fp, "arglist");
      for (c2 = 1; c2 < c1; c2++)
	fprintf(fp, ")");
      fprintf(fp, ");\n");
    }
}
	
void
Class::outputBodyOfCaseClause(FILE *fp, SLList<Method *> *ml, int firstarg)
{
  while (ml->length() > 0)
    {
      Method *m1 = ml->remove_front();
      SLList<Method *> *tl = getMethodsWithIdenticalArgumentTo(m1, ml, firstarg);
      tl->prepend(m1);
      if (ml->length() == 0)
	{
	  // if all of the methods have just been removed from ml,
	  // put in an "ASSERT" -- that argument #<firstarg> must be identical
	  // to m1's firstarg

	  outputAssertForArgument(fp, m1, firstarg);
	}
      else
	{
	  // otherwise, we have to recurse, and output two or more if statements.

	  outputIfForArgument(fp, m1, firstarg);
	}
      
      if (tl->length() == 1)
	{
	  // if there is only one element on tl, it means we can assert all
	  // the rest of the arguments past firstarg, extract the arguments,
	  // make the function call, and return a value.
      
	  m1 = tl->remove_front();
	  outputAssertsForArgumentsAfter(fp, m1, firstarg);
	  outputArgumentExtractions(fp, m1);
	  outputFunctionCall(fp, m1);
	  outputMutationsToSchemeArguments(fp, m1); /* should only need to be used
						       to mutate floats when passed
						       by reference */
	  outputReturnValue(fp, m1);
	}
      else
	{
	  // there is more than one method that has the same type
	  // at argument #<firstarg>. So recurse on tl, incrementing
	  // firstarg.

	  outputBodyOfCaseClause(fp, tl, firstarg + 1);
	}

      if (ml->length() > 0)
	{
	  // close the if statement we started above
	  fprintf(fp, "}\n");
	}

      delete tl;
    }
}

void
Class::outputAssertForArgument(FILE *fp, Method *m1, int whicharg)
{
  Argument *arg;
  Pix placeholder;
  int counter = 1;
  placeholder = m1->args->first();
  while (counter != whicharg)
    {
      m1->args->next(placeholder);
      counter++;
    }
  if (!placeholder)
    {
      cerr << "Class::outputAssertForArgument: method " << m1->name << " of class " << name <<
	" has fewer arguments than expected" << endl;
      return;
    }
  arg = (*(m1->args))(placeholder);

  char *pred = getPredicateFromArgument(arg);

  fprintf(fp, "ASSERT(%s(arg%d), arg%d, %d, \"SCM_%s_", pred, whicharg, whicharg, whicharg, name);
  
  // output which method this is
  
  if (m1->name == NULL)
    {
      // method is a constructor
      fprintf(fp, "Constructor");
    }
  else
    {
      char *lower_mname = stringToLowercase(m1->name);
      fprintf(fp, "%s", lower_mname);
      if (m1->operator_num)
	{
	  fprintf(fp, "%s", OperatorList[m1->operator_num - 1]);
	}
      delete[] lower_mname;
    }
  
  fprintf(fp, "\");\n");
}

char *
Class::getPredicateFromArgument(Argument *arg)
{
  // if argument is a float,
  // assert that the input must be a number
  // (allow integers and floats to match float arguments)
  // if argument is a reference to a float,
  // assert that the input must be a float,
  // since integers can not be mutated.

  if (!strcmp(arg->td->type, "float"))
    {
      if (arg->td->is_array == 0)
	{
	  if (arg->td->ref == REFERENCE)
	    {
	      // FLOATP must be added somewhere
	      // defined as FLOATP(x) = (NIMP(x) && NUMP(x))

	      char *ret = new char[strlen("FLOATP") + 1];
	      strcpy(ret, "FLOATP");
	      return ret;
	    }
	  else
	    {
	      char *ret = new char[strlen("NUMBERP") + 1];
	      strcpy(ret, "NUMBERP");
	      return ret;
	    }
	}
    }

  // if argument is an int,
  // assert that it must be an int
  
  if (!strcmp(arg->td->type, "int"))
    {
      if (arg->td->is_array == 0)
	{
	  char *ret = new char[strlen("INUMP") + 1];
	  strcpy(ret, "INUMP");
	  return ret;
	}
    }
      
  // if argument is a double,
  // assert that the Scheme representation must be a float.
  if (!strcmp(arg->td->type, "double"))
    {
      if (arg->td->is_array == 0)
	{
	  char *ret = new char[strlen("FLOATP") + 1];
	  strcpy(ret, "FLOATP");
	  return ret;
	}
    }

  // if argument type is a char ptr,
  // assert that argument must be a string

  if ((!strcmp(arg->td->type, "char")) && (arg->td->ref == STRING) && (arg->td->depth == 1))
    {
      char *ret = new char[strlen("STRINGP") + 1];
      strcpy(ret, "STRINGP");
      return ret;
    }

  // if argument is a char,
  // assert that the argument must be an immediate char
  if ((!strcmp(arg->td->type, "char")) && ((arg->td->ref == OBJECT) ||
					   (arg->td->ref == REFERENCE)))
    {
      char *ret = new char[strlen("ICHRP") + 1];
      strcpy(ret, "ICHRP");
      return ret;
    }

  if (arg->td->is_array)
    {
      // argument input must be a vector.
      // output VECTORP as the predicate.
      // however, this must be modified in the
      // case of multi-dimensional arrays to recursively
      // verify that all members of initial vector are themselves vectors
      // (probably should happen at a higher level)
      char *ret = new char[strlen("VECTORP") + 1];
      strcpy(ret, "VECTORP");
      return ret;
    }
  
  // otherwise, assume that the predicate should be the
  // data type of the argument, in uppercase, with "P" appended.

  char *tmp = stringToUppercase(arg->td->type);
  char *ret = new char[strlen(tmp) + 2];
  strcpy(ret, tmp);
  ret[strlen(tmp) + 1] = 0;
  ret[strlen(tmp)] = 'P';
  delete[] tmp;
  return ret;
}

void
Class::outputIfForArgument(FILE *fp, Method *m1, int whicharg)
{
  Argument *arg;
  Pix placeholder;
  int counter = 1;
  placeholder = m1->args->first();
  while (counter != whicharg)
    {
      m1->args->next(placeholder);
      counter++;
    }
  if (!placeholder)
    {
      cerr << "Class::outputIfForArgument: method " << m1->name << " of class " << name <<
	" has fewer arguments than expected" << endl;
      return;
    }
  arg = (*(m1->args))(placeholder);

  char *pred = getPredicateFromArgument(arg);

  fprintf(fp, "if (%s(arg%d))\n", pred, whicharg);
  fprintf(fp, "{\n");
}

void
Class::outputAssertsForArgumentsAfter(FILE *fp, Method *m1, int whicharg)
{
  int counter;

  for (counter = whicharg + 1; counter <= m1->args->length(); counter++)
    {
      outputAssertForArgument(fp, m1, counter);
    }
}

void
Class::outputArgumentExtractions(FILE *fp, Method *m1)
{
  Pix p1;
  Argument *arg;
  p1 = m1->args->first();
  
  for (int counter = 1; counter <= m1->args->length(); counter++)
    {
      arg = (*(m1->args))(p1);
      if (m1->name)
	outputArgumentExtractionNumber(fp, arg, counter, m1->name, m1->operator_num);
      else
	outputArgumentExtractionNumber(fp, arg, counter, "Constructor");
      m1->args->next(p1);
    }
}

void
Class::outputArgumentExtractionNumber(FILE *fp, Argument *arg, int whicharg,
				      char *method_name, int operator_num)
{
  // output the data type of the argument

  if ((!strcmp(arg->td->type, "int")) && (arg->td->is_array == 0))
    {
      if (arg->td->is_long)
	fprintf(fp, "long ");
      if (arg->td->is_short)
	fprintf(fp, "short ");
      fprintf(fp, "int int%d = INUM(arg%d);\n", whicharg, whicharg);
      return;
    }

  if (!strcmp(arg->td->type, "float") && (arg->td->is_array == 0))
    {
      // output conditional number extraction. 
      // (i.e. if passed value is an int, promote it)
     
      fprintf(fp, "float float%d = ((INUMP(arg%d)) ? (INUM(arg%d)) : (REAL(arg%d)));\n",
	      whicharg, whicharg, whicharg, whicharg);
      return;
    }

  if (!strcmp(arg->td->type, "double") && (arg->td->is_array == 0))
    {
      // output conditional number extraction. 
      // (i.e. if passed value is an int, promote it)
     
      if (arg->td->is_long)
	fprintf(fp, "long ");
      if (arg->td->is_short)
	fprintf(fp, "short ");
      fprintf(fp, "double double%d = ((INUMP(arg%d)) ? (INUM(arg%d)) : (REAL(arg%d)));\n",
	      whicharg, whicharg, whicharg, whicharg);
      return;
    }

  if ((!strcmp(arg->td->type, "char")) && (arg->td->ref == STRING) && (arg->td->depth == 1))
    {
      // passed value is a string
      // extract it like this

      fprintf(fp, "char *char%d = CHARS(arg%d);\n", whicharg, whicharg);
      return;
    }

  if ((!strcmp(arg->td->type, "char")) && (arg->td->ref == OBJECT))
    {
      fprintf(fp, "char char%d = ICHR(arg%d);\n", whicharg, whicharg);
      return;
    }

  if (((arg->td->ref == REFERENCE) || (arg->td->ref == OBJECT) || (arg->td->ref == POINTER_TO_OBJECT)) && (arg->td->is_array == 0))
    {
      // output extraction based on object's type

      // output the data type of the extracted object first
      // recall that all C++ objects are stored as pointers in Scheme

      char *lowertype = stringToLowercase(arg->td->type);
      char *uppertype = stringToUppercase(arg->td->type);
      if (arg->td->is_struct)
	fprintf(fp, "struct ");
      fprintf(fp, "%s *%s%d = SCM2%s(arg%d);\n", arg->td->type, lowertype, whicharg, uppertype, whicharg);
      delete[] lowertype;
      delete[] uppertype;
      return;
    }

  if (arg->td->is_array > 0)
    {
      int c1;
      // output declaration of the array to extract into

      // output the data type
      if (arg->td->is_short)
	fprintf(fp, "short ");
      if (arg->td->is_long)
	fprintf(fp, "long ");
      if (arg->td->is_struct)
	fprintf(fp, "struct ");
      fprintf(fp, "%s ", arg->td->type);
//      if (arg->td->ref == REFERENCE)
//	fprintf(fp, "(&");
      // output the array name
      fprintf(fp, "array%d", whicharg);
//      if (arg->td->ref == REFERENCE)
//	fprintf(fp, ")");
      // output the array indices
      for (c1 = 0; c1 < arg->td->is_array; c1++)
	{
	  if (arg->td->array_dimensions == 0)
	    cerr << "Class::outputArgumentExtractionNumber: " <<
	      "unspecified array dimension (not supported)" << endl;
	  fprintf(fp, "[%d]", arg->td->array_dimensions[c1]);
	}
      fprintf(fp, ";\n");
      // output counters and temporaries for array extraction
      for (c1 = 0; c1 < arg->td->is_array; c1++)
	{
	  fprintf(fp, "int count%dd%d;\n", c1+1, whicharg);
	  fprintf(fp, "SCM temp%dd%d;\n", c1+1, whicharg);
	}
      // output array extraction
      outputArrayExtractionForArgNumber(fp, arg, whicharg, method_name, operator_num, 1);
    }
}

void
Class::outputArrayExtractionForArgNumber(FILE *fp, Argument *arg,
					 int whicharg, char *method_name, int operator_num,
					 int whichindex)
{
  // output for loop for this dimension
  fprintf(fp, "for (count%dd%d = 0; count%dd%d < %d; count%dd%d++)\n",
	  whichindex, whicharg, whichindex, whicharg, arg->td->array_dimensions[whichindex-1],
	  whichindex, whicharg);
  fprintf(fp, "{\n");

  if (whichindex == 1)
    {
      fprintf(fp, "temp%dd%d = vector_ref(arg%d, MAKINUM(count%dd%d));\n",
	      whichindex, whicharg, whicharg, whichindex, whicharg);
    }
  else
    {
      fprintf(fp, "temp%dd%d = vector_ref(temp%dd%d, MAKINUM(count%dd%d));\n",
	      whichindex, whicharg, whichindex-1, whicharg, whichindex, whicharg);
    }

  if (whichindex < arg->td->is_array)
    {
      fprintf(fp, "ASSERT(VECTORP(temp%dd%d), temp%dd%d, %d, \"SCM_%s_",
	      whichindex, whicharg, whichindex, whicharg, whicharg, name);

      // output which method this is
  
      if (method_name == NULL)
	{
	  // method is a constructor
	  fprintf(fp, "Constructor");
	}
      else
	{
	  char *lower_mname = stringToLowercase(method_name);
	  fprintf(fp, "%s", lower_mname);
	  if (operator_num)
	    {
	      fprintf(fp, "%s", OperatorList[operator_num - 1]);
	    }
	  delete[] lower_mname;
	}
      fprintf(fp, "\");\n");

      outputArrayExtractionForArgNumber(fp, arg, whicharg, method_name, operator_num,
					whichindex + 1);
      // close for loop
      fprintf(fp, "}\n");
      return;
    }

  if (whichindex == arg->td->is_array)
    {
      // output assignment of extracted value from array
      // output assert
      fprintf(fp, "ASSERT(");

      if (!strcmp(arg->td->type, "float"))
	{
	  if (arg->is_constant)  // vector will not be mutated
	    fprintf(fp, "NUMBERP");
	  else  // vector will be mutated later
	    fprintf(fp, "FLOATP");
	}
      else
	{
	  if (!strcmp(arg->td->type, "int"))
	    {
	      fprintf(fp, "INUMP");
	    }
	  else
	    {
	      char *uppername = stringToUppercase(arg->td->type);
	      fprintf(fp, "%sP", uppername);
	      delete[] uppername;
	    }
	}

      fprintf(fp, "(temp%dd%d), arg%d, %d, \"SCM_%s_",
	      whichindex, whicharg, whicharg, whicharg, name);
      
      // output which method this is
  
      if (method_name == NULL)
	{
	  // method is a constructor
	  fprintf(fp, "Constructor");
	}
      else
	{
	  char *lower_mname = stringToLowercase(method_name);
	  fprintf(fp, "%s", lower_mname);
	  if (operator_num)
	    {
	      fprintf(fp, "%s", OperatorList[operator_num - 1]);
	    }
	  delete[] lower_mname;
	}
      fprintf(fp, "\");\n");


      // extract element from vector
      fprintf(fp, "array%d", whicharg);

      int c1;
      for (c1 = 0; c1 < arg->td->is_array; c1++)
	{
	  fprintf(fp, "[count%dd%d]", c1+1, whicharg);
	}
      fprintf(fp, " = ");
      
      if (!strcmp(arg->td->type, "int"))
	{
	  fprintf(fp, "INUM(temp%dd%d);\n", whichindex, whicharg);
	}
      else
	{
	  if (!strcmp(arg->td->type, "float"))
	    {
	      fprintf(fp, "((INUMP(temp%dd%d)) ? (INUM(temp%dd%d)) : REAL(temp%dd%d));\n",
		      whichindex, whicharg, whichindex, whicharg, whichindex, whicharg);
	    }
	  else
	    {
	      char *uppername = stringToUppercase(arg->td->type);
	      fprintf(fp, "(*(SCM2%s(temp%dd%d)));\n", uppername, whichindex, whicharg);
	      delete[] uppername;
	    }
	}
      // close the for loop
      fprintf(fp, "}\n");
    }
}

void
Class::outputFunctionCall(FILE *fp, Method *m1)
{
  // Keep in mind we're dealing with an interpreter.
  // For all methods that return objects (i.e. not references
  // or pointers), we must allocate memory from the heap, to
  // ensure it stays around in the C side of things.

  if (m1->name == NULL)  // constructor
    {
      fprintf(fp, "%s *retval = (%s *)must_malloc(sizeof(%s), \"%s\");\n",
	      m1->td->type, m1->td->type, m1->td->type, m1->td->type);
      fprintf(fp, "retval = new (retval) %s(", m1->td->type);
//      fprintf(fp, "%s *retval = new %s(", m1->td->type, m1->td->type);
      // output arguments to constructor
      outputCallingArgumentsForMethod(fp, m1);
      fprintf(fp, ");\n");
      // append this pointer onto the GC list
      if (hasDestructor())
	{
	  fprintf(fp, "int key = (int) retval;\n");
	  fprintf(fp, "int found;\n");
	  fprintf(fp, "HashTableAddEntry(SCM_%s_gc_table, (HashKey) key, &found);\n", m1->td->type);
	  fprintf(fp, "if (found)\n");
	  fprintf(fp, "cerr << \"ERROR: %s \" << (void *) retval << \" already in the GC hash table\" << endl;\n", name);
	}
      return;
    }
  else
    {
      char *lowername1 = stringToLowercase(name);

      if (m1->td->ref == OBJECT)
	{
	  if (strcmp(m1->td->type, "void"))  // don't allocate anything for void
	    {
	      if (!strcmp(m1->td->type, "int"))
		{
		  if (m1->is_constant)
		    fprintf(fp, "const ");
		  if (m1->td->is_short)
		    fprintf(fp, "short ");
		  if (m1->td->is_long)
		    fprintf(fp, "long ");
		  fprintf(fp, "int ");
		}
	      else if (!strcmp(m1->td->type, "float") ||
		       !strcmp(m1->td->type, "double"))
		{
		  if (m1->is_constant)
		    fprintf(fp, "const ");
		  fprintf(fp, "float ");
		}
	      else if (!strcmp(m1->td->type, "char"))
		{
		  if (m1->is_constant)
		    fprintf(fp, "const ");
		  fprintf(fp, "char ");
		}
	      else
		{
		  // allocate memory for returned object
		  if (m1->td->is_struct)
		    fprintf(fp, "struct ");
		  if (!m1->represents_class_variable)
		    {
		      fprintf(fp, "%s *retval = new %s", m1->td->type, m1->td->type);
		      fprintf(fp, ";\n");
		      fprintf(fp, "*");
		    }
		  else
		    {
		      fprintf(fp, "%s *", m1->td->type);
		    }
		}
	      fprintf(fp, "retval = ");
	    }
	  if (m1->td->is_enum)
	    {
	      // Need to cast enumerated data type back to int.

	      fprintf(fp, "(int) ");
	    }
	  if (m1->operator_num)  // operator function call
	    {
	      if (m1->is_static)
		{
		  fprintf(fp, "%s::%s%s", name, m1->name, OperatorList[m1->operator_num - 1]);
		}
	      else if (m1->is_friend)
		{
		  fprintf(fp, "%s%s", m1->name, OperatorList[m1->operator_num - 1]);
		}
	      else
		{
		  fprintf(fp, "%s1->%s%s", lowername1, m1->name, OperatorList[m1->operator_num - 1]);
		}
	    }	      
	  else
	    {
	      if (m1->represents_class_variable)
		{
		  if (m1->td->ref == OBJECT)
		    {
		      if (strcmp(m1->td->type, "int") && 
			  strcmp(m1->td->type, "float") &&
			  strcmp(m1->td->type, "double") &&
			  strcmp(m1->td->type, "char"))
			fprintf(fp, "&");
		    }
		}
	      if (m1->is_static)
		{
		  fprintf(fp, "%s::%s", name, m1->name);
		}
	      else
		{
		  fprintf(fp, "%s1->%s", lowername1, m1->name);
		}
	    }
	  if (!m1->represents_class_variable)
	    fprintf(fp, "(");
	  outputCallingArgumentsForMethod(fp, m1);
	  if (!m1->represents_class_variable)
	    fprintf(fp, ")");
	  fprintf(fp, ";\n");
	  delete[] lowername1;
	  return;
	}
      else
	{
	  if (m1->is_constant)
	    fprintf(fp, "const ");
	  if (m1->td->is_short)
	    fprintf(fp, "short ");
	  if (m1->td->is_long)
	    fprintf(fp, "long ");
	  if (m1->td->is_struct)
	    fprintf(fp, "struct ");
	  fprintf(fp, "%s ", m1->td->type);
	  if ((m1->td->ref == POINTER) || (m1->td->ref == REFERENCE_TO_POINTER) || (m1->td->ref == POINTER_TO_OBJECT) || (m1->td->ref == STRING))
	    fprintf(fp, "*");
	  if ((m1->td->ref == REFERENCE) || (m1->td->ref == REFERENCE_TO_POINTER) || (m1->td->ref == POINTER_TO_OBJECT))
	    {
	      if (m1->td->is_array)
		fprintf(fp, "(");
	      if (m1->td->ref != POINTER_TO_OBJECT)
		fprintf(fp, "&");
	    }
	  fprintf(fp, "retval");
	  if ((m1->td->ref == REFERENCE) || (m1->td->ref == REFERENCE_TO_POINTER) || (m1->td->ref == POINTER_TO_OBJECT))
	    {
	      if (m1->td->is_array)
		fprintf(fp, ")");
	    }
//	  if (m1->td->is_array && ((m1->td->ref == POINTER_TO_OBJECT) || (m1->td->ref == REFERENCE_TO_POINTER)))
//	    fprintf(fp, "*");
	  if (m1->td->is_array && ((m1->td->ref == REFERENCE) || (m1->td->ref == POINTER_TO_OBJECT)))
	    {
	      for (int c1 = 0; c1 < m1->td->is_array; c1++)
		{
		  fprintf(fp, "[%d]", m1->td->array_dimensions[c1]);
		}
	    }
	  fprintf(fp, " = ");
	  if (m1->operator_num)  // operator function call
	    {
	      if (m1->is_static)
		{
		  fprintf(fp, "%s::%s%s(", name, m1->name, OperatorList[m1->operator_num - 1]);
		}
	      else if (m1->is_friend)
		{
		  fprintf(fp, "%s%s(", m1->name, OperatorList[m1->operator_num - 1]);
		}
	      else
		{
		  fprintf(fp, "%s1->%s%s(", lowername1, m1->name, OperatorList[m1->operator_num - 1]);
		}
	    }	      
	  else
	    {
	      if (m1->is_static)
		{
		  fprintf(fp, "%s::%s", name, m1->name);
		  if (!m1->represents_class_variable)
		    fprintf(fp, "(");
		}
	      else
		{
		  fprintf(fp, "%s1->%s", lowername1, m1->name);
		  if (!m1->represents_class_variable)
		    fprintf(fp, "(");
		}
	    }
	  outputCallingArgumentsForMethod(fp, m1);
	  if (!m1->represents_class_variable)
	    fprintf(fp, ")");
	  fprintf(fp, ";\n");
	  delete[] lowername1;
	  return;
	}
    }
}

void
Class::outputCallingArgumentsForMethod(FILE *fp, Method *m1)
{
  Pix p1;
  Argument *arg;
  p1 = m1->args->first();
  int counter;
  
  if ((m1->is_static) || (m1->name == NULL) || (m1->is_friend))  /* static method,
								    constructor,
								    or friend operator */
    {
      counter = 1;
    }
  else
    {
      counter = 2;
      m1->args->next(p1);
    }

  while (p1)
    {
      arg = (*(m1->args))(p1);
      
      if ((!strcmp(arg->td->type, "int")) && (arg->td->is_array == 0))
	{
	  if (arg->td->ref == POINTER_TO_OBJECT)
	    fprintf(fp, "&");
	  if (arg->td->is_enum)
	    {
	      // Need to add scoping information for enums defined in classes
	      // without explicit scoping given in Types file.

	      if ((!parent_header->parent_directory->isScopedDataType(arg->td->enumerated_type)) && (arg->td->is_nonscoped == 0))
		{
		  fprintf(fp, "(%s::%s) ", name, arg->td->enumerated_type);
		}
	      else
		{
		  fprintf(fp, "(%s) ", arg->td->enumerated_type);
		}
	    }
	  fprintf(fp, "int%d", counter);
	}
      else if ((!strcmp(arg->td->type, "float")) && (arg->td->is_array == 0))
	{
	  fprintf(fp, "float%d", counter);
	}
      else if ((!strcmp(arg->td->type, "double")) && (arg->td->is_array == 0))
	{
	  fprintf(fp, "double%d", counter);
	}
      else if ((!strcmp(arg->td->type, "char")) && (arg->td->ref == STRING) && (arg->td->depth == 1))
	{
	  fprintf(fp, "char%d", counter);
	}
      else if ((!strcmp(arg->td->type, "char")) && ((arg->td->ref == OBJECT) ||
						    (arg->td->ref == REFERENCE)) && (arg->td->is_array == 0))
	{
	  fprintf(fp, "char%d", counter);
	}
      else if (arg->td->is_array == 0)
	{
	  if ((arg->td->ref == REFERENCE) || (arg->td->ref == OBJECT))
	    {
	      fprintf(fp, "*");
	    }
	  else
	    {
	      if (arg->td->depth > 1)
		cerr << "Class::outputCallingArgumentsForMethod: " << 
		  "pointer depths of > 1 not yet supported" << endl;
	    }
	  
	  char *lowertype = stringToLowercase(arg->td->type);
	  fprintf(fp, "%s%d", lowertype, counter);
	  delete[] lowertype;
	}
      else
	{
	  if (arg->td->is_array > 0)
	    {
	      fprintf(fp, "array%d", counter);
	    }
	}
      
      m1->args->next(p1);
      if (p1)
	fprintf(fp, ", ");
      counter++;
    }
}

void
Class::outputMutationsToSchemeArguments(FILE *fp, Method *m1)
{
  Pix p1;
  Argument *arg;
  int counter;
  int c1;

  counter = 1;
  p1 = m1->args->first();
  while (p1)
    {
      arg = (*(m1->args))(p1);
      
      if ((!strcmp(arg->td->type, "float")) && 
	  (arg->td->ref == REFERENCE) && 
	  (arg->td->is_array == 0) &&
	  (!arg->is_constant))
	{
	  // mutate this floating point argument

	  fprintf(fp, "SCM_Mutate_float(arg%d, float%d);\n", counter, counter);
	}
      else if ((arg->td->is_array) && (!arg->is_constant))
	{
	  // output counters and temporaries for array extraction
	  for (c1 = 0; c1 < arg->td->is_array; c1++)
	    {
	      fprintf(fp, "int mucount%dd%d;\n", c1+1, counter);
	      fprintf(fp, "SCM mutemp%dd%d;\n", c1+1, counter);
	    }

	  outputArrayMutationForArgNumber(fp, arg, counter, m1->name, 1);

	  // need to copy array back into Scheme vector.
	}
      m1->args->next(p1);
      counter++;
    }
}
	
void
Class::outputArrayMutationForArgNumber(FILE *fp, Argument *arg,
					 int whicharg, char *method_name, 
					 int whichindex)
{
  // we have already done all necessary ASSERTs for
  // this passed vector during the extraction phase.
  // skip them this time. Mutate vector according
  // to values in array.

  // output for loop for this dimension
  fprintf(fp, "for (mucount%dd%d = 0; mucount%dd%d < %d; mucount%dd%d++)\n",
	  whichindex, whicharg, whichindex, whicharg, arg->td->array_dimensions[whichindex-1],
	  whichindex, whicharg);
  fprintf(fp, "{\n");

  if (whichindex == 1)
    {
      fprintf(fp, "mutemp%dd%d = vector_ref(arg%d, MAKINUM(mucount%dd%d));\n",
	      whichindex, whicharg, whicharg, whichindex, whicharg);
    }
  else
    {
      fprintf(fp, "mutemp%dd%d = vector_ref(mutemp%dd%d, MAKINUM(mucount%dd%d));\n",
	      whichindex, whicharg, whichindex-1, whicharg, whichindex, whicharg);
    }

  if (whichindex < arg->td->is_array)
    {
      outputArrayMutationForArgNumber(fp, arg, whicharg, method_name,
				      whichindex + 1);
      // close for loop
      fprintf(fp, "}\n");
      return;
    }

  if (whichindex == arg->td->is_array)
    {
      // mutate value in vector to value in array.

      if (!strcmp(arg->td->type, "float"))
	{
	  // mutate floating point value

	  fprintf(fp, "SCM_Mutate_float(mutemp%dd%d, array%d", whichindex, whicharg, whicharg);
	  int c1;
	  for (c1 = 0; c1 < arg->td->is_array; c1++)
	    {
	      fprintf(fp, "[mucount%dd%d]", c1+1, whicharg);
	    }
	  fprintf(fp, ");\n");
	}
      else
	{
	  if (!strcmp(arg->td->type, "int"))
	    {
	      // mutate integer value.
	      // note this must be done using vector_set
	      // since integers are not pointers in SCM.

	      fprintf(fp, "vector_set(mutemp%dd%d, MAKINUM(mucount%dd%d), MAKINUM(array%d",
		      whichindex, whicharg, whichindex, whicharg, whicharg);
	      int c1;
	      for (c1 = 0; c1 < arg->td->is_array; c1++)
		{
		  fprintf(fp, "[mucount%dd%d]", c1+1, whicharg);
		}
	      fprintf(fp, "));\n");
	    }
	  else
	    {
	      if (!parent_header->parent_directory->isBasicDataType(arg->td->type))
		{
		  // mutate object using memberwise copy

		  char *uppername = stringToUppercase(arg->td->type);
		  fprintf(fp, "*(SCM2%s(mutemp%dd%d)) = array%d", 
			  uppername, whichindex, whicharg, whicharg);
		  int c1;
		  for (c1 = 0; c1 < arg->td->is_array; c1++)
		    {
		      fprintf(fp, "[mucount%dd%d]", c1+1, whicharg);
		    }
		  fprintf(fp, ";\n");
		}
	      else
		{
		  cerr << "Class::outputArrayMutationForArgNumber: " << 
		    "currently array type \"" << arg->td->type << "\" is unsupported" << endl;
		}
	    }
	}
      // close the for loop
      fprintf(fp, "}\n");
    }
}

void
Class::outputReturnValue(FILE *fp, Method *m1)
{
  if (!strcmp(m1->td->type, "void") && (m1->td->ref == OBJECT))
    {
      fprintf(fp, "return UNSPECIFIED;\n");
      return;
    }

  if (m1->td->is_array > 0)
    {
      // reconstruct vector out of retval
      outputArrayToVectorConversion(fp, m1, 1);
/*
      int c1;
      for (c1 = 1; c1 <= is_array; c1++)
	{
*/	  
//      cerr << "Class::outputReturnValue: arrays not supported yet" << endl;
      return;
    }

  if (m1->name == NULL)
    {
      // constructor.

      fprintf(fp, "return SCM_%s(retval);\n", name);
      return;
    }

  if ((!strcmp(m1->td->type, "int")) && ((m1->td->ref == OBJECT) ||
					 (m1->td->ref == REFERENCE)) && (m1->td->is_array == 0))
    {
      fprintf(fp, "return MAKINUM(retval);\n");
      return;
    }

  if (((!strcmp(m1->td->type, "float")) ||
       (!strcmp(m1->td->type, "double"))) && ((m1->td->ref == OBJECT) ||
					      (m1->td->ref == REFERENCE)) && (m1->td->is_array == 0))
    {
      fprintf(fp, "return makdbl(retval, 0.0);\n");
      return;
    }

  if ((!strcmp(m1->td->type, "char")) && ((m1->td->ref == STRING)))
    {
      fprintf(fp, "return makfromstr(retval, strlen(retval));\n");
      return;
    }

  if ((!strcmp(m1->td->type, "char")) && (m1->td->ref == OBJECT))
    {
      fprintf(fp, "return MAKICHR(retval);\n");
      return;
    }

  fprintf(fp, "return SCM_%s(retval);\n", m1->td->type);
  return;
}

void
Class::outputArrayToVectorConversion(FILE *fp, Method *m1, int whichindex)
{
  fprintf(fp, "SCM scmtemp%dd;\n", whichindex);
  fprintf(fp, "int scmcount%dd;\n", whichindex);
  fprintf(fp, "scmtemp%dd = make_vector(MAKINUM(%d), UNDEFINED);\n", whichindex, m1->td->array_dimensions[whichindex - 1]);
  fprintf(fp, "for (scmcount%dd = 0; scmcount%dd < %d; scmcount%dd++)\n", whichindex, whichindex, m1->td->array_dimensions[whichindex - 1], whichindex);
  fprintf(fp, "{\n");

  if (whichindex < m1->td->is_array)
    {
      outputArrayToVectorConversion(fp, m1, whichindex + 1);
    }
  else
    {
      fprintf(fp, "vector_set(scmtemp%dd, MAKINUM(scmcount%dd), ", whichindex, whichindex);
      if (!strcmp(m1->td->type, "int"))
	{
	  fprintf(fp, "MAKINUM");
	}
      else if (!strcmp(m1->td->type, "float"))
	{
	  fprintf(fp, "makdbl");
	}
      else
	{
	  fprintf(fp, "SCM_%s", m1->td->type);
	}
      fprintf(fp, "(retval");
      int counter;
      for (counter = 1; counter <= m1->td->is_array; counter++)
	{
	  fprintf(fp, "[scmcount%dd]", counter);
	}
      if (!strcmp(m1->td->type, "float"))
	{
	  fprintf(fp, ", 0.0");
	}
      fprintf(fp, "));\n");
    }

  fprintf(fp, "}\n");

  if (whichindex > 1)
    fprintf(fp, "vector_set(scmtemp%dd, MAKINUM(scmcount%dd), scmtemp%dd);\n", whichindex - 1, whichindex - 1, whichindex);
  else
    fprintf(fp, "return scmtemp1d;\n");
}

int
Class::argumentsAreIndistinguishable(Argument *arg1, Argument *arg2)
{
  if (strcmp(arg1->td->type, arg2->td->type)) // different data types
    return 0;

  if (((arg1->td->is_array == 0) && (arg2->td->is_array != 0)) ||
      ((arg1->td->is_array != 0) && (arg2->td->is_array == 0)))
    return 0;

  return 1;
}

int
Class::methodsAreIndistinguishable(Method *m1, Method *m2)
{
  if (m1->args->length() != m2->args->length())   // trivial case, should never happen
    return 0;

  Pix p1, p2;
  Argument *arg1, *arg2;
  p1 = m1->args->first();
  p2 = m2->args->first();
  while (p1 && p2)
    {
      arg1 = (*(m1->args))(p1);
      arg2 = (*(m2->args))(p2);
      if (!(argumentsAreIndistinguishable(arg1, arg2)))
	return 0;
      m1->args->next(p1);
      m1->args->next(p2);
    }
	
  return 1;
}

SLList<Method *> *
Class::getMethodsWithIdenticalArgumentTo(Method *m1, SLList<Method *> *ml, int whicharg)
{
  SLList<Method *> *tml = new SLList<Method *>;
  int result;
  Pix p1, p2;
  Method *front, *ptr;
  int c1;
  Argument *arg1, *arg2;

 repeat:
  result = ml->remove_front(front);
  if (result)
    {
      if (front->args->length() != m1->args->length())
	{
	  cerr << "Class::getMethodsWithIdenticalArgumentTo: methods front and m1 have different argument list lengths" << endl;
	  return tml;
	}

      if (whicharg > m1->args->length())
	{
	  cerr << "Class::getMethodsWithIdenticalArgumentTo: whicharg is too big (" << whicharg << 
	    ", maximum is " << m1->args->length() << ")" << endl;
	  return tml;
	}
      
      // check to see if this method matches the search criterion
      p1 = front->args->first();
      p2 = m1->args->first();
      c1 = 1;
      while (c1 < whicharg && (p1 && p2))
	{
	  front->args->next(p1);
	  m1->args->next(p2);
	  c1++;
	}
      arg1 = (*(front->args))(p1);
      arg2 = (*(m1->args))(p2);
      if (argumentsAreIndistinguishable(arg1, arg2))
	{
	  tml->append(front);
	  // go back to removing the front of the list (since as
	  // yet we can't tell when we've gone through the entire list)
	  goto repeat;
	}
      else
	{
	  // append this method back on the end
	  ml->append(front);
	  ptr = ml->remove_front();
	  while (ptr != front)
	    {
	      if (ptr->args->length() != m1->args->length())
		{
		  cerr << "Class::getMethodsWithIdenticalArgumentTo: methods ptr and m1 have different argument list lengths" << endl;
		  return tml;
		}
	      
	      if (whicharg > m1->args->length())
		{
		  cerr << "Class::getMethodsWithIdenticalArgumentTo: whicharg is too big (" << whicharg << 
		    ", maximum is " << m1->args->length() << ")" << endl;
		  return tml;
		}
	      
	      // check to see if this method matches the search criterion
	      p1 = ptr->args->first();
	      p2 = m1->args->first();
	      c1 = 1;
	      while (c1 < whicharg && (p1 && p2))
		{
		  ptr->args->next(p1);
		  m1->args->next(p2);
		  c1++;
		}
	      arg1 = (*(ptr->args))(p1);
	      arg2 = (*(m1->args))(p2);
	      if (argumentsAreIndistinguishable(arg1, arg2))
		{
		  tml->append(ptr);
		}
	      else
		{
		  ml->append(ptr);
		}
	      ptr = ml->remove_front();
	    }
	  ml->prepend(ptr);  // keep list in order
	}
      return tml;
    }
  else
    {
      // commented out because this is not an error
//      cerr << "Class::getMethodsWithIdenticalArgumentTo: method list was empty" << endl;
      return tml;
    }
}

void Class::forceUserToChooseOneMethod(SLList<Method *> *ml)
{
  int choice;

  cout << "In class " << name << ":" << endl;
  cout << "Multiple indistinguishable methods. Please choose one:" << endl;
  cout << endl;
  Pix p1;
  Method *m1;
  int c1 = 1;
  p1 = ml->first();
  while (p1)
    {
      m1 = (*ml)(p1);
      cout << c1 << ". ";
      m1->print();
      c1++;
      ml->next(p1);
    }

  choice = getUserChoice("Your choice: ", 1, ml->length());

  // remove all methods but the user's choice

  Method *keep;

  c1 = 1;
  keep = ml->remove_front();

  while (c1 < choice)
    {
      keep = ml->remove_front();
      c1++;
    }

  // method we want to keep is now stored in "keep"
  // flush out the rest of the methods in ml

  while (ml->length() > 0)
    {
      m1 = ml->remove_front();
    }

  // add "keep" back onto ml
  
  ml->append(keep);
}

int
Class::getUserChoice(char *message, int minchoice, int maxchoice)
{
  int choice;
  int result;

  do
    {
      printf("%s", message);
      result = scanf("%d", &choice);
      if (result == 0)
	{
	  printf("Sorry, you must enter a number between %d and %d.\n",
		 minchoice, maxchoice);
	}
      if (result == EOF)
	{
	  printf("Sorry, you must enter a number between %d and %d.\n",
		 minchoice, maxchoice);
	}	  
	fflush(stdin);
      if ((choice < minchoice) || (choice > maxchoice))
	printf("Error: your choice must be between %d and %d.\n", minchoice, maxchoice);
    }
  while ((choice < minchoice) || (choice > maxchoice));
  return choice;
}
