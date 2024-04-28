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

#ifndef _CLASSCONTROLLER_
#define _CLASSCONTROLLER_

#include <stdio.h>
#include <string.h>
#include "SLList.h"
//#include "SCM_Globals.h"

#define NAME_LENGTH 40

class ClassNode;
class ClassController;

// Node in the class hierarchy structure.

class ClassNode {
public:
  SLList<ClassNode *> *parents;
  char *class_name;
  long class_identity;
  long (*dispatch_func)(char *method_name, long arglist);
  SLList<ClassNode *> *children;
  ClassNode(const char *my_class_name) { 
    parents = new SLList<ClassNode *>;
    children = new SLList<ClassNode *>;
    class_identity = 0;
    dispatch_func = NULL;
    class_name = new char[strlen(my_class_name) + 1];
    strcpy(class_name, my_class_name);
  }
  ClassNode(long my_identity) { 
    parents = new SLList<ClassNode *>;
    children = new SLList<ClassNode *>;
    class_identity = my_identity;
    dispatch_func = NULL;
    class_name = NULL;
  }
  ~ClassNode() { 
    delete parents;
    delete children;
    if (class_name) delete[] class_name;
  }
};

// Object which manages a class hierarchy.

class ClassController {
public:
  enum ClassControllerMessage {
    MSG_NO_ERROR,
    ERR_PARENT_CLASSES_NOT_ADDED,
    ERR_CLASS_ALREADY_ADDED
    };
    
  ClassController() {
    base_classes = new SLList<ClassNode *>;
    nodes = new SLList<ClassNode *>;
    using_longs = -1;
    name_array = 0;
    identity_array = 0;
    node_array = 0;
    arrays_built = 0;
  }
  ClassNode *addClassToHierarchy(char **parent_classes,
				 char *class_name,
				 int add_as_base_class = 0);
  ClassNode *addClassToHierarchy(long *parent_classes,
				 long class_identity,
				 int add_as_base_class = 0);
  ClassNode *addClassToHierarchy(char **parent_classes,
				 long class_identity,
				 char *class_name,
				 int add_as_base_class = 0);
  ClassNode *addClassToHierarchy(long *parent_classes,
				 long class_identity,
				 char *class_name,
				 int add_as_base_class = 0);

  // The following two methods will be used while
  // building up the class hierarchy out of the
  // directory tree
  ClassNode *searchDownForClass(char *class_name);
  ClassNode *searchDownForClass(long class_identity);

  // The next two methods are what allows a subclass
  // to 'know' what its parent classes are
  ClassNode *searchUpForClass(ClassNode *superclass_node,
			      char *subclass_name);
  ClassNode *searchUpForClass(ClassNode *superclass_node,
			      long subclass_identity);

  // This builds a sorted list of type identifiers (only
  // when using longs) and their associated nodes in the
  // class hierarchy. Makes searchUpForClass possible.
  // *Should* only be called once (when all classes are added
  // to the hierarchy), although I suppose it could be
  // called more than once if necessary
  void buildSortedClassList();
  void printSortedList();

  void printHierarchy();
  void printHierarchyToFile(FILE *fp);
  ClassControllerMessage getLastError();

  friend long SCM_Global_Dispatch(long arglist, ...);
  friend long SCM_CPlusPlus_ObjectP(long arglist, ...);
  friend class DirectoryTree;
protected:
  SLList<ClassNode *> *base_classes;
  SLList<ClassNode *> *nodes; // holds all nodes
  char * *name_array;
  long *identity_array;
  long *node_array;  // holds addresses of node pointers
  int arrays_built;
  int using_longs; /* possible values:
		      -1 : not initialized
		       0 : using only strings
		       1 : using longs
		       2 : using both strings and longs
		       */
  ClassControllerMessage last_error;
  ClassNode * _searchDownForClass(ClassNode *base, char *class_name);
  ClassNode * _searchDownForClass(ClassNode *base, long class_identity);
  ClassNode * _searchUpForClass(ClassNode *superclass_node, ClassNode *subclass_node);
  ClassNode * binarySearchByName(char *class_name);
  ClassNode * binarySearchByType(long class_identity);
  ClassNode * _binarySearchByName(char *class_name, int start, int end);
  ClassNode * _binarySearchByType(long class_identity, int start, int end);
  void quicksortByType();
  void quicksortByName();
  void _quicksortByType(long *a, char **b, long *n, int l, int r);
  void _quicksortByName(long *a, char **b, long *n, int l, int r);
};

#endif

    
