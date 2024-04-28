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

// Definitions for the following data structures:
// Header, Class, Method, Variable, Argument, TypeDesc

#ifndef _HEADER_
#define _HEADER_

#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include "SLList.h"

class Header;
class DirectoryTree;
class TypeDef;

// utility functions
int isNonEssentialKeyword(char *);
int operatorLookup(char *);

// utility macros
#define LETTER(c)  (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z') || (c) == '_')
#define LETTERORNUMBER(c) (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z') || ((c) >= '0' && (c) <= '9') || ((c) == '_'))
#define NUMBER(c)  (((c) >= '0') && (((c) <= '9')))

enum ArgType {
  OBJECT,
  REFERENCE,
  POINTER,
  REFERENCE_TO_POINTER,
  POINTER_TO_OBJECT,
  STRING
  };

// Complete C++ operator list -- Stroustrup p. 226

const char OperatorList[40][7] = {
  "+",
  "-",
  "*",
  "/",
  "%",
  "^",
  "&",
  "|",
  "~",
  "!",
  "=",
  "<",
  ">",
  "+=",
  "-=",
  "*=",
  "/=",
  "%=",
  "^=",
  "&=",
  "|=",
  "<<",
  ">>",
  ">>=",
  "<<=",
  "==",
  "!=",
  "<=",
  ">=",
  "&&",
  "||",
  "++",
  "--",
  "->*",
  ",",
  "->",
  "[]",
  "()",
  "new",
  "delete"
};
			       
// Scheme representations for the above operators.
// All of these are the same as above except for new, delete, (), [].

const char SchemeOperatorList[40][10] = {
  "+",
  "-",
  "*",
  "/",
  "%",
  "^",
  "&",
  "|",
  "~",
  "!",
  "=",
  "<",
  ">",
  "+=",
  "-=",
  "*=",
  "/=",
  "%=",
  "^=",
  "&=",
  "|=",
  "<<",
  ">>",
  ">>=",
  "<<=",
  "==",
  "!=",
  "<=",
  ">=",
  "&&",
  "||",
  "++",
  "--",
  "->*",
  ",",
  "->",
  "-brackets",
  "-parens",
  "-new",
  "-delete"
};
			       
// Class encapsulating a type description.

class TypeDesc {
public:
  char *type;
  ArgType ref;
  int depth;
  int is_array;
  int is_long;
  int is_short;
  int is_struct;

  // following three definitions are for enumerated data types
  int is_enum;
  int is_nonscoped;
  char *enumerated_type;

  int array_dimensions[5];

  TypeDesc() { type = NULL; ref = OBJECT; depth = 0; is_array = 0; is_long = 0; is_short = 0; is_struct = 0; is_enum = 0; is_nonscoped = 0; enumerated_type = NULL; }
  ~TypeDesc() { if (type) delete[] type; }
};

// Class describing a class variable.

class Variable {
public:
  char *name;
  TypeDesc *td;
  int is_static;
  int is_constant;  /* some class variables can be initialized
		       only in the constructor */
  
  Variable() { name = NULL; td = new TypeDesc; is_static = 0; is_constant = 0; }
  ~Variable() { if (name) delete[] name; }
};

// Class describing an argument to a method.

class Argument {
public:
  TypeDesc *td;
  int is_constant;
  int is_optional;

  Argument() { td = new TypeDesc; is_constant = 0; is_optional = 0; }
  ~Argument() {}
};

// Class describing a member function of a class.

class Method {
public:
  char *name;
  TypeDesc *td;
  int is_static;
  int is_constant;
  int is_friend;
  int represents_class_variable;
  SLList<Argument *> *args;
  int operator_num;

  Method() { name = NULL; td = new TypeDesc; 
	     args = new SLList<Argument *>; 
	     operator_num = 0; is_static = 0; 
	     is_constant = 0; is_friend = 0; 
	     represents_class_variable = 0; }
  ~Method() { if (name) delete[] name; 
	      Pix placeholder;
	      placeholder = args->first();
	      while (placeholder)
		{
		  Argument *arg = (*args)(placeholder);
		  delete arg;
		  args->next(placeholder);
		}
	      delete args; }
  void print()
    {
      int counter;

      if (is_static)
	cout << "static ";
      if (is_constant)
	cout << "const ";
      cout << td->type << " ";
      if ((td->ref == POINTER) || 
	  (td->ref == REFERENCE_TO_POINTER) ||
	  (td->ref == POINTER_TO_OBJECT) ||
	  (td->ref == STRING))
	for (counter = 0; counter < td->depth; counter++)
	  cout << "*";
      if ((td->ref == REFERENCE) || (td->ref == REFERENCE_TO_POINTER))
	cout << "&";
      if (operator_num)
	cout << name << OperatorList[operator_num - 1];
      else
	if (name)
	  cout << name;
      if (td->is_array)
	{
	  for (counter = 0; counter < td->is_array; counter++)
	    cout << "[" << td->array_dimensions[counter] << "]";
	}
      cout << "(";

      Argument *aptr;
      Pix p1;
      p1 = args->first();

      while (p1)
	{
	  aptr = (*(args))(p1);
	  if (aptr->is_constant)
	    cout << "const ";
	  cout << aptr->td->type;
	  if ((aptr->td->ref == POINTER) || 
	      (aptr->td->ref == REFERENCE_TO_POINTER) ||
	      (aptr->td->ref == POINTER_TO_OBJECT) ||
	      (aptr->td->ref == STRING))
	    for (counter = 0; counter < aptr->td->depth; counter++)
	      cout << "*";
	  if ((aptr->td->ref == REFERENCE) || (aptr->td->ref == REFERENCE_TO_POINTER))
	    cout << "&";
	  if (aptr->td->is_array)
	    {
	      for (counter = 0; counter < aptr->td->is_array; counter++)
		cout << "[" << aptr->td->array_dimensions[counter] << "]";
	    }
	  args->next(p1);
	  if (p1)
	    cout << ", ";
	}
      cout << ");" << endl;
    }      
};

enum LineType {
  VARIABLE,
  METHOD
  };

// Class describing a line of text from a header file.

class Line {
public:
  SLList<char *> *tokens;
  LineType type;

  Line() { tokens = new SLList<char *>; type = VARIABLE; }
  ~Line() { Pix placeholder;
	    placeholder = tokens->first();
	    while (placeholder)
	      {
	       char *tok = (*tokens)(placeholder);
	       delete[] tok;
	       tokens->next(placeholder);
	     }
	    delete tokens; }
  void addToken(char *token) {  char *token_buf = new char[strlen(token) + 1];
				strcpy(token_buf, token);
				tokens->append(token_buf); }
};

// Class describing the class structure.
// This class knows how to generate glue code for a
// Scheme interface to the class it describes.

class Class {
public:
  char *name;
  SLList<char *> *superclasses;
  SLList<Method *> *methods;
  SLList<Variable *> *variables;

  Header *parent_header;

  // hack to allow opaque function callbacks to be
  // described using this data structure
  int exists_as_pointer_only;

  //
  // Constructor.
  //
  Class();

  //
  // Destructor. Tries to reclaim malloced memory.
  //
  ~Class();
  
  //
  // Sets the parent header file where this class is defined.
  //
  void setHeader(Header *hd);

  //
  // Adds a line from a header file to the class.
  //
  void addLine(Line *line);

  //
  // Conditionally adds a method to this class's method list.
  // Ignores the method if it is in the parent directory's Ignores list.
  //
  void appendToMethods(Method *method);
  
  //
  // Print this class to the screen.
  //
  void print();

  //
  // Returns true if this class has a public constructor/destructor
  // defined in its header file.
  //
  int hasConstructor();
  int hasDestructor();

  //
  // Outputs a header file for the glue code for this class.
  // 
  void outputHeaderFile(FILE *fp);

  //
  // Outputs the glue code (written in C++) defining the SCM
  // backend for a Scheme interface to this class.
  // 

  void outputSchemeInterface(FILE *fp);

private:

  // used by addLine
  char *conditionalUnscopeName(char *a_name);

  // functions used by outputHeaderFile:

  // Outputs function prototypes for all converted methods/variables.
  void outputPrototypes(FILE *fp);
  // Outputs prototypes for class variables.
  void outputPrototypesForClassVariables(FILE *fp);
  // Outputs prototypes for member functions
  void outputPrototypesForMethods(FILE *fp);
  
  // functions used by outputSchemeInterface:

  // Outputs standard variables for the class:
  // which parent classes it has, its type code, etc.
  void outputStandardDeclarations(FILE *fp);

  // Outputs alphabetically sorted member functions and
  // function to initialize all the member functions.
  // These are combined to save time.
  void outputSortedIprocsAndInitializationProcedure(FILE *fp);
  void quicksortIprocs(char **a, char **b, int l, int r);

  //
  // Outputs standard functions used by all classes.
  //

  void outputStandardSCMFunctions(FILE *fp);
  
  // functions used by outputStandardSCMFunctions:

  // Outputs function to GC (delete) this class type if
  // collected by SCM. This must only happen if the object
  // was directly allocated from SCM.
  void outputFreeProcedure(FILE *fp);

  // Outputs function to print this object
  void outputPrintProcedure(FILE *fp);

  // Function to determine whether two objects are equal.
  // This is just a pointer comparison right now.
  void outputEqualPProcedure(FILE *fp);

  // Function to type cast a SMOB to an object of this type.
  void outputCastProcedure(FILE *fp);

  // Predicate function for this object type
  void outputPredicateProcedure(FILE *fp);

  // Functions to convert a pointer or reference to an object
  // of this type into its SCM representation.
  void outputConversionProcedures(FILE *fp);
  
  // Function which gives Scheme a notion of a null pointer.
  // Currently special cased to only be generated for type "void".
  void outputNullProcedure(FILE *fp);

  // Function that knows about the valid member functions and class
  // variables for this class. Used with ClassController to allow
  // C++-like calling of member functions of objects from within SCM.
  void outputDispatchProcedure(FILE *fp);

  // Resolves pointer ambiguities. A pointer can be a pointer to an
  // object, a pointer to the start of a fixed-length array, or a pointer
  // to the start of a null-terminated array. These ambiguities must
  // be resolved by the user.
  void resolvePointers();

  //
  // Resolves all types used in this class with the typedefs
  // defined for the parent directory.
  //
  void resolveTypeDefs();
  
  // functions used by resolveTypeDefs:

  // merges a typedef (defined in DirectoryTree.h) with a
  // typedesc of a variable, argument, method, etc.
  void mergeTypeDescWithTypeDef(TypeDesc *tds, TypeDef *tdf);

  //
  // Prepends a "this" pointer to argument lists of all methods
  // which need it.
  //
  void prependObjectPointerToArgumentLists();

  //
  // Function which converts class variables into methods so
  // the compiler can treat them the same later on.
  //
  void convertClassVariablesToMethods();

  //
  // Functions which recursively expand methods with optional arguments 
  // at the end of their parameter lists into versions with non-optional
  // arguments, and add the newly generated methods to the method list.
  //
  void expandOptionalArgumentMethods();
  void expandMethodWithOptionalArgs(Method *m1);

  //
  // Function which outputs the backend for a Scheme constructor
  // if a publically defined constructor was found for this class.
  //
  void outputConstructor(FILE *fp);

  //
  // This function is the base call to begin compiling all the class's
  // member functions into glue code.
  //
  void outputMethods(FILE *fp);

  // functions used by outputMethods:

  // Function which destructively modifies the method list and
  // returns all methods with the same name as the requested one.
  // Operator_num is passed to differentiate amongst overloaded operators.
  SLList<Method *> *getMethodsByName(char *method_name, int operator_num = 0);

  // Returns the longest argument list length of the list of (possibly
  // overloaded) functions.
  int getLongestArgListLength(SLList<Method *> *mlist);

  // Outputs the header for the glue code function for this method.
  void outputFunctionHeaderAndSwitch(FILE *fp, char *methodname, int operator_num, int maxargs);

  // Outputs the default: clause in the case statement for this method.
  // Returns cleanly to the interpreter with an error.
  void outputDefaultCaseClause(FILE *fp, char *method_name, int operator_num);

  // Destructively modifies the passed method list "ml" and returns
  // all methods with the desired argument list length.
  SLList<Method *> *getMethodsOfArgLength(SLList<Method *> *ml, int len);

  // Function which verifies that the all methods in the passed list
  // are distinguishable from each other.
  void verifyMethodsAreDistinguishable(SLList<Method *> *ml);

  // Outputs the case clause for the condition that the parameter list
  // passed in from Scheme is of length argslen.
  void outputMethodCaseClause(FILE *fp, int argslen, SLList<Method *> *ml);
  // Outputs simple code to extract the SCM objects from the passed
  // parameter list from Scheme.
  void outputArgListExtractions(FILE *fp, int argslen);

  // Outputs the body of the case clause. Uses the functions below.
  void outputBodyOfCaseClause(FILE *fp, SLList<Method *> *ml, int firstarg);

  // Assert that argument whicharg being passed from Scheme must be
  // of the type specified in m1.
  void outputAssertForArgument(FILE *fp, Method *m1, int whicharg);
  // Gets the predicate for use in the above assert.
  char *getPredicateFromArgument(Argument *arg);

  // Output an if statement for argument whicharg.
  // Used in the case of overloaded functions.
  void outputIfForArgument(FILE *fp, Method *m1, int whicharg);

  // No more possibilities for overloading this function;
  // all passed parameters from Scheme after argument whicharg must
  // match the function prototype exactly.
  void outputAssertsForArgumentsAfter(FILE *fp, Method *m1, int whicharg);

  // Outputs code to extract C++ data types from SCM objects.
  void outputArgumentExtractions(FILE *fp, Method *m1);
  // Outputs extraction for a given argument number.
  void outputArgumentExtractionNumber(FILE *fp, Argument *arg, int whicharg, char *method_name, int operator_num = 0);
  // Outputs a vector-to-array conversion for argument number whicharg.
  void outputArrayExtractionForArgNumber(FILE *fp, Argument *arg, int whicharg, char *method_name, int operator_num, int whichindex);

  // Outputs the C++ function call trying to be made from Scheme.
  void outputFunctionCall(FILE *fp, Method *m1);
  // Outputs the arguments for this function call.
  void outputCallingArgumentsForMethod(FILE *fp, Method *m1);
  // Outputs code to mutate Scheme arguments if C++ specifies that
  // passed arguments may be mutated.
  void outputMutationsToSchemeArguments(FILE *fp, Method *m1);
  // Outputs code to mutate a Scheme vector to have the same value
  // as a C++ array possibly mutated by a function call.
  void outputArrayMutationForArgNumber(FILE *fp, Argument *arg,
				       int whicharg, char *method_name, 
				       int whichindex);
  // Converts the return value from this function call into SCM format.
  void outputReturnValue(FILE *fp, Method *m1);
  // Outputs an array-to-vector conversion if the method returns an array.
  void outputArrayToVectorConversion(FILE *fp, Method *m1, int whichindex);
  
  //
  // Utility functions
  //

  // Returns 1 if two arguments are indistinguishable to Scheme, 0 otherwise.
  int argumentsAreIndistinguishable(Argument *arg1, Argument *arg2);
  // Returns 1 if two methods are indistinguishable to Scheme, 0 otherwise.
  // Uses the above procedure.
  int methodsAreIndistinguishable(Method *m1, Method *m2);
  // Function which destructively modifies ml and returns all methods in ml
  // which have an identical argument number whicharg to method m1.
  SLList<Method *> *getMethodsWithIdenticalArgumentTo(Method *m1, SLList<Method *> *ml, int whicharg);
  // Function which destructively modifies ml. Forces the user to choose
  // one of multiple overloaded functions indistinguishable to Scheme.
  void forceUserToChooseOneMethod(SLList<Method *> *ml);
  // Utility function to get a user's choice between minchoice and maxchoice.
  int getUserChoice(char *message, int minchoice, int maxchoice);
};

// Class which describes a file in terms of tokens. 

class File {
public:
  SLList<char *> *tokens;

  File() { tokens = new SLList<char *>; }
  ~File() { Pix placeholder;
	    placeholder = tokens->first();
	    while (placeholder)
	      {
		char *tok = (*tokens)(placeholder);
		delete[] tok;
		tokens->next(placeholder);
		}
	    delete tokens; }
  void printTokens();
  void addToken(char *token) {  char *token_buf = new char[strlen(token) + 1];
				strcpy(token_buf, token);
				tokens->append(token_buf); }
};

// Class which represents a header file in terms of the classes it contains.

class Header {
public:
  char *filename;

  Header();
  Header(const char* origfilename);
  ~Header();

  // Sets the parent directory where this header is contained.
  void setDirectoryTree(DirectoryTree *dt);

  // Sets the file name of this header file.
  void setFileName(const char *newfilename);

  // Reads the header file in.
  // Does tokenizing of input file and parsing into class structures
  // by using buildClassList, below.
  void readFromFile();

  // Prints all classes contained in this header file
  void print();

  // List of all classes defined in this header file
  SLList<Class *> *classes;

  // Parent directory where this header file is found
  DirectoryTree *parent_directory;

  friend class DirectoryTree;
private:
  // Returns a newly malloced string without the .* extension.
  char *stringWithoutSuffix(const char* filename);

  // Builds the list of classes from the read in file.
  void buildClassList();

  // Conditionally adds a_class to the list of classes if its
  // name isn't in the parent directory's Ignores list.
  void appendToClasses(Class *a_class);

  // Name of this header file
  char *name;

  // Used internally to store all the tokens in this header file.
  File *file;
};


#endif
