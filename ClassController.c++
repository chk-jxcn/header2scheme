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
#include "ClassController.h"

#define BIG_DEBUG 0

ClassNode *
ClassController::addClassToHierarchy(char **parent_classes,
				     char *class_name,
				     int add_as_base_class)
{
  int valid_flag = 1;
  ClassNode *node = new ClassNode(class_name);

  if (using_longs == -1)
    using_longs = 0;
  else
    if (using_longs != 0)
      {
	cerr << "ClassController::addClassToHierarchy: can't switch to using strings as type identifiers" << endl;
	return NULL;
      }

  if (!add_as_base_class)
    {
      int counter = 0;
      ClassNode *parent_node;

      if (searchDownForClass(class_name) != 0)
	{
	  cerr << "ClassController::addClassToHierarchy: class \"";
	  cerr << class_name << "\" is already in the hierarchy" << endl;
	  valid_flag = 0;
	}
      else
	{
	  char *ptr = parent_classes[counter++];
	  while (ptr)
	    {
	      if ((parent_node = searchDownForClass(ptr)) == 0)
		{
//		  cerr << "ClassController::addClassToHierarchy: parent class \"";
//		  cerr << ptr << "\" of class \"" << class_name << "\" is not";
//		  cerr << " in the class hierarchy" << endl;
		  valid_flag = 0;
		}
	      else
		{
		  node->parents->append(parent_node);
		}
	      ptr = parent_classes[counter++];
	    }
	}
    }

  if (valid_flag)
    {
      if (node->parents->length() > 0)
	{
	  Pix placeholder;
	  ClassNode *ptr;
	  
	  placeholder = node->parents->first();
	  while (placeholder)
	    {
	      ptr = (*(node->parents))(placeholder);
	      ptr->children->append(node);
	      node->parents->next(placeholder);
	    }
	}
      else
	{
	  base_classes->append(node);
	}
      nodes->append(node);
      arrays_built = 0;
      return node;
    }

  return NULL;
}

ClassNode *
ClassController::addClassToHierarchy(long *parent_classes,
				     long class_identity,
				     int add_as_base_class)
{
  int valid_flag = 1;
  ClassNode *node = new ClassNode(class_identity);

  if (using_longs == -1)
    using_longs = 1;
  else
    if (using_longs != 1)
      {
	cerr << "ClassController::addClassToHierarchy: can't switch to using longs as type identifiers" << endl;
	return NULL;
      }

  if (!add_as_base_class)
    {
      int counter = 0;
      ClassNode *parent_node;

      if (searchDownForClass(class_identity) != 0)
	{
	  cerr << "ClassController::addClassToHierarchy: class \"";
	  cerr << class_identity << "\" is already in the hierarchy" << endl;
	  valid_flag = 0;
	}
      else
	{
	  long class_id = parent_classes[counter++];
	  while (class_id)
	    {
	      if ((parent_node = searchDownForClass(class_id)) == 0)
		{
//		  cerr << "ClassController::addClassToHierarchy: parent class \"";
//		  cerr << class_id << "\" of class \"" << class_identity << "\" is not";
//		  cerr << " in the class hierarchy" << endl;
		  valid_flag = 0;
		}
	      else
		{
		  node->parents->append(parent_node);
		}
	      class_id = parent_classes[counter++];
	    }
	}
    }

  if (valid_flag)
    {
      if (node->parents->length() > 0)
	{
	  Pix placeholder;
	  ClassNode *ptr;
	  
	  placeholder = node->parents->first();
	  while (placeholder)
	    {
	      ptr = (*(node->parents))(placeholder);
	      ptr->children->append(node);
	      node->parents->next(placeholder);
	    }
	}
      else
	{
	  base_classes->append(node);
	}
      nodes->append(node);
      arrays_built = 0;
      return node;
    }

  return NULL;
}

ClassNode *
ClassController::addClassToHierarchy(char **parent_classes,
				     long class_identity,
				     char *class_name,
				     int add_as_base_class)
{
  int valid_flag = 1;
  ClassNode *node = new ClassNode(class_name);
  node->class_identity = class_identity;

  if (using_longs == -1)
    using_longs = 2;
  else
    if (using_longs != 2)
      {
	cerr << "ClassController::addClassToHierarchy: can't switch to using both strings and longs" << endl;
	return NULL;
      }

  if (!add_as_base_class)
    {
      int counter = 0;
      ClassNode *parent_node;

      if (searchDownForClass(class_identity) != 0)
	{
	  cerr << "ClassController::addClassToHierarchy: class \"";
	  cerr << class_name << "\" is already in the hierarchy" << endl;
	  valid_flag = 0;
	}
      else
	{
	  char *ptr = parent_classes[counter++];
	  while (ptr)
	    {
	      if ((parent_node = searchDownForClass(ptr)) == 0)
		{
//		  cerr << "ClassController::addClassToHierarchy: parent class \"";
//		  cerr << ptr << "\" of class \"" << class_name << "\" is not";
//		  cerr << " in the class hierarchy" << endl;
		  valid_flag = 0;
		}
	      else
		{
		  node->parents->append(parent_node);
		}
	      ptr = parent_classes[counter++];
	    }
	}
    }

  if (valid_flag)
    {
      if (node->parents->length() > 0)
	{
	  Pix placeholder;
	  ClassNode *ptr;
	  
	  placeholder = node->parents->first();
	  while (placeholder)
	    {
	      ptr = (*(node->parents))(placeholder);
	      ptr->children->append(node);
	      node->parents->next(placeholder);
	    }
	}
      else
	{
	  base_classes->append(node);
	}
      nodes->append(node);
      arrays_built = 0;
      return node;
    }

  return NULL;
}


ClassNode *
ClassController::addClassToHierarchy(long *parent_classes,
				     long class_identity,
				     char *class_name,
				     int add_as_base_class)
{
  int valid_flag = 1;
  ClassNode *node = new ClassNode(class_name);
  node->class_identity = class_identity;

  if (using_longs == -1)
    using_longs = 2;
  else
    if (using_longs != 2)
      {
	cerr << "ClassController::addClassToHierarchy: can't switch to using both strings and longs" << endl;
	return NULL;
      }

  if (!add_as_base_class)
    {
      int counter = 0;
      ClassNode *parent_node;

      if (searchDownForClass(class_name) != 0)
	{
	  cerr << "ClassController::addClassToHierarchy: class \"";
	  cerr << class_name << "\" is already in the hierarchy" << endl;
	  valid_flag = 0;
	}
      else
	{
	  long class_id = parent_classes[counter++];
	  while (class_id)
	    {
	      if ((parent_node = searchDownForClass(class_id)) == 0)
		{
//		  cerr << "ClassController::addClassToHierarchy: parent class \"";
//		  cerr << class_id << "\" of class \"" << class_name << "\" is not";
//		  cerr << " in the class hierarchy" << endl;
		  valid_flag = 0;
		}
	      else
		{
		  node->parents->append(parent_node);
		}
	      class_id = parent_classes[counter++];
	    }
	}
    }

  if (valid_flag)
    {
      if (node->parents->length() > 0)
	{
	  Pix placeholder;
	  ClassNode *ptr;
	  
	  placeholder = node->parents->first();
	  while (placeholder)
	    {
	      ptr = (*(node->parents))(placeholder);
	      ptr->children->append(node);
	      node->parents->next(placeholder);
	    }
	}
      else
	{
	  base_classes->append(node);
	}
      nodes->append(node);
      arrays_built = 0;
      return node;
    }

  return NULL;
}

ClassNode *
ClassController::searchDownForClass(char *class_name)
{
  Pix placeholder;
  ClassNode *base;
  ClassNode *return_node;

  if ((using_longs != 0) && (using_longs != 2))
    {
      cerr << "ClassController::searchDownForClass: can't search on strings, no information present" << endl;
      return NULL;
    }

  placeholder = base_classes->first();
  while (placeholder)
    {
      base = (*base_classes)(placeholder);
      return_node = _searchDownForClass(base, class_name);
      if (return_node)
	return return_node;
      base_classes->next(placeholder);
    }

  return NULL;
}

ClassNode *
ClassController::searchDownForClass(long class_identity)
{
  Pix placeholder;
  ClassNode *base;
  ClassNode *return_node;

  if ((using_longs != 1) && (using_longs != 2))
    {
      cerr << "ClassController::searchDownForClass: can't search on longs, no information present" << endl;
      return NULL;
    }

  placeholder = base_classes->first();
  while (placeholder)
    {
      base = (*base_classes)(placeholder);
      return_node = _searchDownForClass(base, class_identity);
      if (return_node)
	return return_node;
      base_classes->next(placeholder);
    }

  return NULL;
}

ClassNode *
ClassController::_searchDownForClass(ClassNode *base, char *class_name)
{
  Pix placeholder;
  ClassNode *child;
  ClassNode *return_node;
  
  if (strcmp(base->class_name, class_name) == 0)
    return base;

  placeholder = base->children->first();
  while (placeholder)
    {
      child = (*(base->children))(placeholder);
      return_node = _searchDownForClass(child, class_name);
      if (return_node)
	return return_node;
      base->children->next(placeholder);
    }

  return NULL;
}

ClassNode *
ClassController::_searchDownForClass(ClassNode *base, long class_identity)
{
  Pix placeholder;
  ClassNode *child;
  ClassNode *return_node;
  
  if (base->class_identity == class_identity)
    return base;

  placeholder = base->children->first();
  while (placeholder)
    {
      child = (*(base->children))(placeholder);
      return_node = _searchDownForClass(child, class_identity);
      if (return_node)
	return return_node;
      base->children->next(placeholder);
    }

  return NULL;
}

void
ClassController::buildSortedClassList()
{
  int length;

  if (using_longs == 1)
    {
      if (identity_array)
	{
	  delete[] identity_array;
	  identity_array = 0;
	}
    }
  else
    {
      if (using_longs == 0)
	{
	  if (name_array)
	    {
	      delete[] name_array;
	      name_array = 0;
	    }
	}
      else
	{
	  if (using_longs == 2)
	    {
	      if (identity_array)
		{
		  delete[] identity_array;
		  identity_array = 0;
		}
	      if (name_array)
		{
		  delete[] name_array;
		  name_array = 0;
		}
	    }
	}
    }
	      
  if (node_array)
    {
      delete[] node_array;
      node_array = 0;
    }

  length = nodes->length();

  if (using_longs)
    identity_array = new long[length];
  if (using_longs == 0 || using_longs == 2)
    {
      name_array = new char *[length];
      for (int counter = 0; counter < length; counter++)
	{
	  name_array[counter] = new char[NAME_LENGTH];
	}
    }

  node_array = new long[length];

  int counter = 0;
  Pix placeholder;
  ClassNode *node;
  placeholder = nodes->first();
  while (placeholder)
    {
      node = (*nodes)(placeholder);
      node_array[counter] = (long) node;
      if (using_longs)
	identity_array[counter] = node->class_identity;
      if (using_longs == 0 || using_longs == 2)
	strcpy(name_array[counter], node->class_name);
      counter++;
      nodes->next(placeholder);
    }
	  
//  cerr << "before sort:" << endl;
//  for (counter = 0; counter < nodes->length(); counter++)
//    {
//      cerr << name_array[counter] << endl;
//    }

  if (using_longs)
    quicksortByType();
  else
    quicksortByName();
  arrays_built = 1;
}

void
ClassController::printSortedList()
{
  if (arrays_built == 0)
    {
      cerr << "ClassController::searchUpForClass: must call buildSortedClassList() first" << endl;
      return;
    }
  cerr << "sorted list:" << endl;
  
  if (using_longs == 0)
    {
      for (int counter = 0; counter < nodes->length(); counter++)
	{
	  cout << name_array[counter] << endl;
	}
    }
  else
    {
      for (int counter = 0; counter < nodes->length(); counter++)
	{
	  cout << identity_array[counter] << endl;
	}
    }
}

void
ClassController::quicksortByType()
{
  _quicksortByType(identity_array, name_array, node_array, 0, nodes->length() - 1);
}

void
ClassController::quicksortByName()
{
  _quicksortByName(identity_array, name_array, node_array, 0, nodes->length() - 1);
}

void
ClassController::_quicksortByType(long *a, char **b, long *n, int l, int r)
{
  int i, j;
  long v, t;
  char *c;

  if (r > l)
    {
      v = a[r]; i = l-1; j = r;
      for (;;)
	{
	  while ((i++ <= r) && (a[i] < v)) ;
	  while ((--j >= 0) && (a[j] > v)) ;
	  if (i >= j) break;
	  t = a[i]; a[i] = a[j]; a[j] = t;
	  t = n[i]; n[i] = n[j]; n[j] = t;
	  if (using_longs == 2)
	    c = b[i]; b[i] = b[j]; b[j] = c;
	}
      t = a[i]; a[i] = a[r]; a[r] = t;
      t = n[i]; n[i] = n[r]; n[r] = t;
      if (using_longs == 2)
	c = b[i]; b[i] = b[r]; b[r] = c;
      _quicksortByType(a, b, n, l, i-1);
      _quicksortByType(a, b, n, i+1, r);
    }
}

void
ClassController::_quicksortByName(long *a, char **b, long *n, int l, int r)
{
  int i, j;
  long t;
  char *c, *v;

  if (r > l)
    {
      v = b[r]; i = l-1; j = r;
      for (;;)
	{
	  while ((++i <= r) && (strcmp(b[i], v) < 0))
	    {
#if BIG_DEBUG
	      cerr << "i comparison of \"" << b[i] << "\" to \"" << v << "\": " << strcmp(b[i], v) << endl;
#endif
	    }
	  while ((--j >= 0) && (strcmp(b[j], v) > 0));
	    {
#if BIG_DEBUG
	      cerr << "j comparison of \"" << b[j] << "\" to \"" << v << "\": " << strcmp(b[j], v) << endl;
#endif
	    }
	  if (i >= j) break;
	  t = n[i]; n[i] = n[j]; n[j] = t;
	  c = b[i]; b[i] = b[j]; b[j] = c;
	}
      t = n[i]; n[i] = n[r]; n[r] = t;
      c = b[i]; b[i] = b[r]; b[r] = c;
      _quicksortByName(a, b, n, l, i-1);
      _quicksortByName(a, b, n, i+1, r);
    }
}

ClassNode *
ClassController::searchUpForClass(ClassNode *superclass_node,
				  char *subclass_name)
{
  if (!arrays_built)
    {
      cerr << "ClassController::searchUpForClass: must call buildSortedClassList() first" << endl;
      return NULL;
    }

  if (using_longs > 0)
    {
      cerr << "ClassController::searchUpForClass: can't search by name when ";
      if (using_longs == 1)
	cerr << "only ";
      cerr << "longs are present" << endl;
      return NULL;
    }

  ClassNode *subclass_node;
  subclass_node = binarySearchByName(subclass_name);
  if (!subclass_node)
    {
//      cerr << "Class \"" << subclass_name << "\" not found in class hierarchy" << endl;
      return NULL;
    }
  return _searchUpForClass(superclass_node, subclass_node);
}
    
ClassNode *
ClassController::searchUpForClass(ClassNode *superclass_node,
				  long subclass_identity)
{
  if (!arrays_built)
    {
      cerr << "ClassController::searchUpForClass: must call buildSortedClassList() first" << endl;
      return NULL;
    }

  if (using_longs == 0)
    {
      cerr << "ClassController::searchUpForClass: can't search by longs when ";
      cerr << "only strings are present" << endl;
      return NULL;
    }

  ClassNode *subclass_node;
  subclass_node = binarySearchByType(subclass_identity);
  if (!subclass_node)
    {
//      cerr << "Class \"" << subclass_identity << "\" not found in class hierarchy" << endl;
      return NULL;
    }
  return _searchUpForClass(superclass_node, subclass_node);
}

ClassNode *
ClassController::_searchUpForClass(ClassNode *superclass_node, ClassNode *subclass_node)
{
#if BIG_DEBUG
  cerr << "Comparing node " << (long) superclass_node << " to " << (long) subclass_node << endl;
#endif

  if (superclass_node == subclass_node)
    {
#if BIG_DEBUG
      cerr << "succeeded" << endl;
#endif
      return superclass_node;
    }

  Pix placeholder;
  ClassNode *node;
  ClassNode *return_node;
  placeholder = subclass_node->parents->first();
  while (placeholder)
    {
      node = (*(subclass_node->parents))(placeholder);
#if BIG_DEBUG
      cerr << "making recursive call with subclass == " << (long) node << endl;
#endif
      return_node = _searchUpForClass(superclass_node, node);
      if (return_node)
	return return_node;
      subclass_node->parents->next(placeholder);
    }
  return NULL;
}

ClassNode *
ClassController::binarySearchByName(char *class_name)
{
  return _binarySearchByName(class_name, 0, nodes->length() - 1);
}

ClassNode *
ClassController::binarySearchByType(long class_identity)
{
  return _binarySearchByType(class_identity, 0, nodes->length() - 1);
}

ClassNode *
ClassController::_binarySearchByName(char *class_name,
				     int start,
				     int end)
{
  int median;
  int result;
  
 repeat:
  if (end < start)
    return NULL;

  if (start == end)
    {
#if BIG_DEBUG
      cerr << "ClassController::_binarySearchByName 1: comparing \"" << class_name;
      cerr << "\" to " << name_array[start] << endl;
#endif

      if (strcmp(class_name, name_array[start]) == 0)
	return (ClassNode *) node_array[start];
      else
	return NULL;
    }

  median = (start + end) >> 1;

#if BIG_DEBUG
  cerr << "ClassController::_binarySearchByName 2: comparing \"" << class_name;
  cerr << "\" to " << name_array[median];
#endif

  result = strcmp(class_name, name_array[median]);
#if BIG_DEBUG
  cerr << ": result = " << result << endl;
#endif
  if (result < 0)
    {
      end = median - 1;
      goto repeat;
    }
  else
    {
      if (result > 0)
	{
	  start = median + 1;
	  goto repeat;
	}
      else
	{
#if BIG_DEBUG
	  cerr << "Return value will be " << node_array[median] << endl;
#endif
	  return (ClassNode *) node_array[median];
	}
    }
}
		  
ClassNode *
ClassController::_binarySearchByType(long class_identity,
				     int start,
				     int end)
{
  int median;
  
 repeat:
  if (end < start)
    return NULL;

  if (start == end)
    {
      if (class_identity == identity_array[start])
	return (ClassNode *) node_array[start];
      else
	return NULL;
    }

  median = (start + end) >> 1;

  if (class_identity < identity_array[median])
    {
      end = median - 1;
      goto repeat;
    }
  else
    {
      if (class_identity > identity_array[median])
	{
	  start = median + 1;
	  goto repeat;
	}
      else
	return (ClassNode *) node_array[median];
    }
}
		  
