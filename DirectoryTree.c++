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

#include "DirectoryTree.h"
#include "Utilities.h"
#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef __sgi
#include <sys/fs/efs_ino.h>
#endif
#include <time.h>

DirectoryTree::DirectoryTree(char *user_directory_name,
			     char *user_package_name,
			     char *user_package_version,
			     char *user_ignores_filename,
			     char *user_types_filename,
			     char *user_opaques_filename,
			     char *user_hints_filename,
			     char *user_bases_filename,
			     char *user_macros_filename,
			     int user_incremental_mode,
			     int user_assume_all_scoped_types_are_ints) :
  COMMAND_LENGTH(256)
{
  headers = new SLList<Header *>;
  global_class_list = new SLList<Class *>;
  classcontroller = new ClassController;

  typedefs = NULL;
  ignores = NULL;
  opaques = NULL;
  hints = NULL;
  bases = NULL;

  directory_name = package_name = package_version = ignores_filename = types_filename = opaques_filename = hints_filename = bases_filename = macros_filename = NULL;

  incremental_mode = user_incremental_mode;
  assume_all_scoped_types_are_ints = user_assume_all_scoped_types_are_ints;
  if (user_directory_name)
    {
      directory_name = new char[strlen(user_directory_name) + 1];
      strcpy(directory_name, user_directory_name);
    }
  if (user_package_name)
    {
      package_name = new char[strlen(user_package_name) + 1];
      strcpy(package_name, user_package_name);
    }
  if (user_package_version)
    {
      package_version = new char[strlen(user_package_version) + 1];
      strcpy(package_version, user_package_version);
    }
  if (user_ignores_filename)
    {
      ignores_filename = new char[strlen(user_ignores_filename) + 1];
      strcpy(ignores_filename, user_ignores_filename);
      ignores = new SLList<Ignore *>;
    }
  if (user_types_filename)
    {
      types_filename = new char[strlen(user_types_filename) + 1];
      strcpy(types_filename, user_types_filename);
      typedefs = new SLList<TypeDef *>;
    }
  if (user_opaques_filename)
    {
      opaques_filename = new char[strlen(user_opaques_filename) + 1];
      strcpy(opaques_filename, user_opaques_filename);
      opaques = new SLList<Opaque *>;
    }
  if (user_hints_filename)
    {
      hints_filename = new char[strlen(user_hints_filename) + 1];
      strcpy(hints_filename, user_hints_filename);
      hints = new SLList<Hint *>;
    }
  if (user_bases_filename)
    {
      bases_filename = new char[strlen(user_bases_filename) + 1];
      strcpy(bases_filename, user_bases_filename);
      bases = new SLList<char *>;
    }
  if (user_macros_filename)
    {
      macros_filename = new char[strlen(user_macros_filename) + 1];
      strcpy(macros_filename, user_macros_filename);
      command_buf = new char[COMMAND_LENGTH];
    }
}

void
DirectoryTree::readTypeDefs()
{
  if (types_filename)
    {
      FILE *tdfp;

      tdfp = fopen(types_filename, "r");
      if (tdfp)
	{
	  char type_buf[256];
	  TypeDef *ntd;

	  // read in original type
	  while (fscanf(tdfp, "%s", type_buf) != EOF)
	    {
	      ntd = new TypeDef;
	      ntd->initial_name = new char[strlen(type_buf + 1)];
	      strcpy(ntd->initial_name, type_buf);

	      // read in the base type for this type
	      if (fscanf(tdfp, "%s", type_buf) == EOF)
		{
		  cerr << "DirectoryTree::readTypeDefs: incomplete base type in typedef" << endl;
		  return;
		}
	      
	      if (!strcmp(type_buf, "enum")) // enumerated data type
		{
		  ntd->resolved_type->type = new char[strlen("int") + 1];
		  strcpy(ntd->resolved_type->type, "int");
		  ntd->resolved_type->is_enum = 1;
		}
	      else if (!strcmp(type_buf, "nonscoped_enum")) /* enum not
							       in class*/
		{
		  ntd->resolved_type->type = new char[strlen("int") + 1];
		  strcpy(ntd->resolved_type->type, "int");
		  ntd->resolved_type->is_enum = 1;
		  ntd->resolved_type->is_nonscoped = 1;
		}
	      else
		{
		  int gotLongShort = 0;
		  if ((!strcmp(type_buf, "long")) ||
		      (!strcmp(type_buf, "short")))
		    {
		      gotLongShort = 1;
		      if (!strcmp(type_buf, "long"))
			ntd->resolved_type->is_long = 1;
		      if (!strcmp(type_buf, "short"))
			ntd->resolved_type->is_short = 1;
		      if (fscanf(tdfp, "%s", type_buf) == EOF)
			{
			  cerr << "DirectoryTree::readTypeDefs: "
			       << "incomplete base type in long/short typedef"
			       << endl;
			  return;
			}
		    }

		  if (gotLongShort &&
		      strcmp(type_buf, "int") &&
		      strcmp(type_buf, "double"))
		    {
		      cerr << "DirectoryTree::readTypeDefs: ERROR: "
			   << "I expected long/short int/double, "
			   << "but instead found long/short "
			   << type_buf << endl;
		      return;
		    }

		  ntd->resolved_type->type = new char[strlen(type_buf) + 1];
		  strcpy(ntd->resolved_type->type, type_buf);

		  // read in reference type
		  if (fscanf(tdfp, "%s", type_buf) == EOF)
		    {
		      cerr << "DirectoryTree::readTypeDefs: incomplete base type in reference" << endl;
		      return;
		    }
		  
		  if (!strcmp(type_buf, "OBJECT"))
		    {
		      ntd->resolved_type->ref = OBJECT;
		    }
		  else if (!strcmp(type_buf, "REFERENCE"))
		    {
		      ntd->resolved_type->ref = REFERENCE;
		    }
		  else if (!strcmp(type_buf, "POINTER"))
		    {
		      ntd->resolved_type->ref = POINTER;
		    }
		  else if (!strcmp(type_buf, "REFERENCE_TO_POINTER"))
		    {
		      ntd->resolved_type->ref = REFERENCE_TO_POINTER;
		    }
		  else if (!strcmp(type_buf, "POINTER_TO_OBJECT"))
		    {
		      ntd->resolved_type->ref = POINTER_TO_OBJECT;
		    }
		  else
		    {
		      cerr << "DirectoryTree::readTypeDefs: invalid reference type \"" <<
			type_buf << "\"" << endl;
		    }

		  if (ntd->resolved_type->ref == POINTER)
		    {
		      if (fscanf(tdfp, "%d", &(ntd->resolved_type->depth)) == EOF)
			{
			  cerr << "DirectoryTree::readTypeDefs: incomplete pointer depth" << endl;
			  return;
			}
		    }
		  else if (ntd->resolved_type->ref == OBJECT)
		    {
		      if (fscanf(tdfp, "%d", &(ntd->resolved_type->is_array)) == EOF)
			{
			  cerr << "DirectoryTree::readTypeDefs: incomplete array depth" << endl;
			  return;
			}
		      
		      if (ntd->resolved_type->is_array)
			{
			  for (int counter = 0; counter < ntd->resolved_type->is_array; counter++)
			    {
			      if (fscanf(tdfp, "%d", &(ntd->resolved_type->array_dimensions[counter])) == EOF)
				{
				  cerr << "DirectoryTree::readTypeDefs: incomplete array dimensions" << endl;
				  return;
				}
			    }
			}
		    }
		}
	      typedefs->append(ntd);
	    }
	}
      else
	{
	  cerr << "DirectoryTree::readTypeDefs: error opening typedef file" << endl;
	}
      fclose(tdfp);
    }
}

void
DirectoryTree::readIgnores()
{
  if (ignores_filename)
    {
      FILE *ifp;

      ifp = fopen(ignores_filename, "r");
      if (ifp)
	{
	  char ign_buf[256];
	  Ignore *ign;

	  while (fscanf(ifp, "%s", ign_buf) != EOF)
	    {
	      ign = new Ignore;

	      if (!strcmp(ign_buf, "DIRECTORY"))
		{
		  ign->type = Ignore::DIRECTORY;
		  if (fscanf(ifp, "%s", ign_buf) != EOF)
		    {
		      ign->data.directory_name = new char[strlen(ign_buf) + 1];
		      strcpy(ign->data.directory_name, ign_buf);
		    }
		  else
		    {
		      cerr << "DirectoryTree::readIgnores: Error reading directory name" << endl;
		      return;
		    }
		}
	      else if (!strcmp(ign_buf, "FILE"))
		{
		  ign->type = Ignore::FILE; 
		  if (fscanf(ifp, "%s", ign_buf) != EOF)
		    {
		      ign->data.file_name = new char[strlen(ign_buf) + 1];
		      strcpy(ign->data.file_name, ign_buf);
		    }
		  else
		    {
		      cerr << "DirectoryTree::readIgnores: Error reading file name " << endl;
		      return;
		    }
		}
	      else if (!strcmp(ign_buf, "CLASS"))
		{
		  ign->type = Ignore::CLASS;
		  if (fscanf(ifp, "%s", ign_buf) != EOF)
		    {
		      ign->data.class_name = new char[strlen(ign_buf) + 1];
		      strcpy(ign->data.class_name, ign_buf);
		    }
		  else
		    {
		      cerr << "DirectoryTree::readIgnores: Error reading class name " << endl;
		      return;
		    }
		}
	      else if (!strcmp(ign_buf, "METHOD"))
		{
		  ign->type = Ignore::METHOD;
		  if (fscanf(ifp, "%s", ign_buf) != EOF)
		    {
		      ign->data.method_ignore.class_name = new char[strlen(ign_buf) + 1];
		      strcpy(ign->data.method_ignore.class_name, ign_buf);
		    }
		  else
		    {
		      cerr << "DirectoryTree::readIgnores: Error reading parent class name of method" << endl;
		      return;
		    }
		  if (fscanf(ifp, "%s", ign_buf) != EOF)
		    {
		      if (!strcmp(ign_buf, "CONSTRUCTOR"))
			{
			  // ignore all constructor methods from this class
			  ign->data.method_ignore.method_name = NULL;
			  ign->data.method_ignore.method_type = new char[strlen(ign->data.method_ignore.class_name) + 1];
			  strcpy(ign->data.method_ignore.method_type, ign->data.method_ignore.class_name);
			}
		      else if (!strcmp(ign_buf, "NAME"))
			{
			  // read out the name of the method
			  if (fscanf(ifp, "%s", ign_buf) != EOF)
			    {
			      ign->data.method_ignore.method_name = new char[strlen(ign_buf) + 1];
			      strcpy(ign->data.method_ignore.method_name, ign_buf);
			    }
			  else
			    {
			      cerr << "DirectoryTree::readIgnores: Error reading method name" << endl;
			      return;
			    }
			}
		      else
			{
			  cerr << "DirectoryTree::readIgnores: Error reading type of method" << endl;
			  return;
			}
		    }
			  
		}
	      else
		{
		  cerr << "DirectoryTree::readIgnores: Error reading ignore data type" << endl;
		  return;
		}
	      ignores->append(ign);
	    }
	}
      else
	{
	  cerr << "DirectoryTree::readIgnores: error opening ignores file" << endl;
	}
    }
}

void
DirectoryTree::readOpaques()
{
  if (opaques_filename)
    {
      FILE *ofp;

      ofp = fopen(opaques_filename, "r");
      if (ofp)
	{
	  char opaque_buf[256];
	  Opaque *op;

	  while (fscanf(ofp, "%s", opaque_buf) != EOF)
	    {
	      op = new Opaque;
	      op->type_name = new char[strlen(opaque_buf) + 1];
	      strcpy(op->type_name, opaque_buf);

	      if (fscanf(ofp, "%s", opaque_buf) != EOF)
		{
		  op->path_name = new char[strlen(opaque_buf) + 1];
		  strcpy(op->path_name, opaque_buf);
		}
	      else
		{
		  cerr << "DirectoryTree::readOpaques: Error reading path name of opaque data type" << endl;
		  return;
		}

	      if (fscanf(ofp, "%d", &(op->exists_as_pointer_only)) == EOF)
		{
		  cerr << "DirectoryTree::readOpaques: Error reading \"exists as pointer only\" field" << endl;
		  return;
		}

	      opaques->append(op);
	    }

	  // now, we want the opaque data types to look exactly the same
	  // as "empty" data types to the rest of the compiler.
	  // what we're going to do is add a bunch of fake header files with
	  // one null class per header to the header list.

	  Pix p1;

	  p1 = opaques->first();
	  while (p1)
	    {
	      op = (*opaques)(p1);

	      Header *head = new Header;
	      head->setDirectoryTree(this);
	      head->setFileName(op->path_name);
	      
	      Class *a_class = new Class;
	      a_class->setHeader(head);
	      a_class->name = op->type_name;
	      a_class->exists_as_pointer_only = op->exists_as_pointer_only;
	      head->classes->append(a_class);
	      headers->append(head);

	      opaques->next(p1);
	    }
	}
      else
	{
	  cerr << "DirectoryTree::readOpaques: Error opening opaques file" << endl;
	}
    }
}

void
DirectoryTree::readHints()
{
  if (hints_filename)
    {
      FILE *hfp;

      hfp = fopen(hints_filename, "r");
      if (hfp)
	{
	  char hints_buf[256];
	  Hint *hint;

	  // read in type of object
	  while (fscanf(hfp, "%s", hints_buf) != EOF)
	    {
	      hint = new Hint;
	      TypeDesc *ntd = new TypeDesc;

	      ntd->type = new char[strlen(hints_buf) + 1];
	      strcpy(ntd->type, hints_buf);

	      // read in reference type
 	      if (fscanf(hfp, "%s", hints_buf) == EOF)
		{
		  cerr << "DirectoryTree::readHints: incomplete base type in reference" << endl;
		  return;
		}
	      
	      if (!strcmp(hints_buf, "OBJECT"))
		{
		  ntd->ref = OBJECT;
		}
	      else if (!strcmp(hints_buf, "REFERENCE"))
		{
		  ntd->ref = REFERENCE;
		}
	      else if (!strcmp(hints_buf, "POINTER"))
		{
		  ntd->ref = POINTER;
		}
	      else if (!strcmp(hints_buf, "REFERENCE_TO_POINTER"))
		{
		  ntd->ref = REFERENCE_TO_POINTER;
		}
	      else if (!strcmp(hints_buf, "POINTER_TO_OBJECT"))
		{
		  ntd->ref = POINTER_TO_OBJECT;
		}
	      else
		{
		  cerr << "DirectoryTree::readHints: invalid reference type \"" <<
		    hints_buf << "\"" << endl;
		}

	      if ((ntd->ref == POINTER) || (ntd->ref == REFERENCE_TO_POINTER) || (ntd->ref == POINTER_TO_OBJECT))
		{
		  if (fscanf(hfp, "%d", &(ntd->depth)) == EOF)
		    {
		      cerr << "DirectoryTree::readHints: incomplete pointer depth" << endl;
		      return;
		    }
		}
	      else if (ntd->ref == OBJECT)
		{
		  if (fscanf(hfp, "%d", &(ntd->is_array)) == EOF)
		    {
		      cerr << "DirectoryTree::readHints: incomplete array depth" << endl;
		      return;
		    }
		  
		  if (ntd->is_array)
		    {
		      for (int counter = 0; counter < ntd->is_array; counter++)
			{
			  if (fscanf(hfp, "%d", &(ntd->array_dimensions[counter])) == EOF)
			    {
			      cerr << "DirectoryTree::readHints: incomplete array dimensions" << endl;
			      return;
			    }
			}
		    }
		}

	      hint->original_typedesc = ntd;

	      if (fscanf(hfp, "%d", &hint->user_choice) == EOF)
		{
		  cerr << "DirectoryTree::readHints: error reading user choice for hint" << endl;
		  return;
		}

	      hints->append(hint);
	    }
	}
      else
	{
	  cerr << "DirectoryTree::readHints: error opening hint file" << endl;
	  return;
	}
    }
}

void
DirectoryTree::readBases()
{
  if (bases_filename)
    {
      FILE *pfp;
      char buf[256];
      
      pfp = fopen(bases_filename, "r");
      if (pfp)
	{
	  while (fscanf(pfp, "%s", buf) != EOF)
	    {
	      char *base = new char[strlen(buf) + 1];
	      strcpy(base, buf);
	      bases->append(base);
	    }
	  fclose(pfp);
	}
      else
	{
	  cerr << "DirectoryTree::readBases: error opening bases file" << endl;
	}
    }
}

int
DirectoryTree::typeDescInHintsList(TypeDesc *td)
{
  Pix p1;
  Hint *h1;
  TypeDesc *t1;

  if (hints)
    {
      p1 = hints->first();
      while (p1)
	{
	  h1 = (*hints)(p1);
	  t1 = h1->original_typedesc;
	  if (!strcmp(td->type, t1->type) &&
	      (td->ref == t1->ref) &&
	      (td->depth == t1->depth) &&
	      (td->is_array == t1->is_array) &&
	      (td->is_long == t1->is_long) &&
	      (td->is_short == t1->is_short) &&
	      (td->is_struct == t1->is_struct))
	    {
	      int flag = 1;
	      for (int counter = 0; counter < td->is_array; counter++)
		{
		  if (td->array_dimensions[counter] != t1->array_dimensions[counter])
		    flag = 0;
		}
	      if (flag)
		return h1->user_choice;
	    }
	  hints->next(p1);
	}
    }
  return 0;
}

int
DirectoryTree::classInBasesList(char *class_name)
{
  Pix p1;
  char *b1;

  if (bases)
    {
      p1 = bases->first();
      while (p1)
	{
	  b1 = (*bases)(p1);
	  if (!strcmp(class_name, b1))
	    return 1;
	  bases->next(p1);
	}
    }
  return 0;
}

int
DirectoryTree::usingMacros()
{
  return ((macros_filename == NULL) ? 0 : 1);
}

char *
DirectoryTree::getMacroCommand(const char *filename)
{
  sprintf(command_buf, "m4 %s %s", macros_filename, filename);
  return command_buf;
}

int
DirectoryTree::fileIsHeader(char *filename)
{
  int length;
  if ((length = strlen(filename)) > 2)
    {
      if ((filename[length - 1] == 'h') &&
	  (filename[length - 2] == '.'))
	return 1;
      if (length > 3)
	{
	  if ((filename[length - 1] == 'h') &&
	      (filename[length - 2] == 'h') &&
	      (filename[length - 3] == '.'))
	    return 1;
	}
    }
  return 0;
}

int
DirectoryTree::directoryInIgnoresList(char *dirname)
{
  Pix p1;
  Ignore *ign;

  if (ignores)
    {
      p1 = ignores->first();
      
      while (p1)
	{
	  ign = (*ignores)(p1);
	  if (ign->type == Ignore::DIRECTORY)
	{
	  if (!strcmp(dirname, ign->data.directory_name))
	    return 1;
	}
	  ignores->next(p1);
	}
    }
  return 0;
}

int
DirectoryTree::fileInIgnoresList(char *filename)
{
  Pix p1;
  Ignore *ign;

  if (ignores)
    {
      p1 = ignores->first();
      
      while (p1)
	{
	  ign = (*ignores)(p1);
	  if (ign->type == Ignore::FILE)
	    {
	      if (!strcmp(filename, ign->data.file_name))
		return 1;
	    }
	  ignores->next(p1);
	}
    }
  return 0;
}

int
DirectoryTree::classInIgnoresList(char *class_name)
{
  Pix p1;
  Ignore *ign;

  if (ignores)
    {
      p1 = ignores->first();
      
      while (p1)
	{
	  ign = (*ignores)(p1);
	  if (ign->type == Ignore::CLASS)
	    {
	      if (!strcmp(class_name, ign->data.class_name))
		return 1;
	    }
	  ignores->next(p1);
	}
    }
  return 0;
}

int
DirectoryTree::methodInIgnoresList(char *class_name,
				   char *method_name,
				   char *method_type)
{
  Pix p1;
  Ignore *ign;

  if (ignores)
    {
      p1 = ignores->first();
      
      while (p1)
	{
	  ign = (*ignores)(p1);
	  if (ign->type == Ignore::METHOD)
	    {
	      if (!strcmp(class_name, ign->data.method_ignore.class_name))
		{
		  if (method_name && ign->data.method_ignore.method_name)
		    {
		      if (!strcmp(method_name, ign->data.method_ignore.method_name))
			return 1;
		    }
		  else
		    {
		      if (!method_name && !ign->data.method_ignore.method_name)
			{
			  if (!strcmp(method_type, ign->data.method_ignore.method_type))
			    return 1;
			}
		    }
		}
	    }
	  ignores->next(p1);
	}
    }
  return 0;
}

void
DirectoryTree::traverseDirectoryTree(char *pathname)
{
  DIR *dirp;
  struct dirent *direntp;
  char tempname[255];
  struct stat statbuf;
  SLList<char *> subdirs;

  if (directoryInIgnoresList(pathname))
    {
      return;
    }

  dirp = opendir(pathname);
  if (!dirp)
    {
      printf("DirectoryTree::traverseDirectoryTree: Error opening directory %s\n", pathname);
      return;
    }
  
  while (direntp = readdir(dirp))
    {
      strcpy(tempname, pathname);
      strcat(tempname, "/");
      strcat(tempname, direntp->d_name);
      if (stat(tempname, &statbuf) == 0)
	{
	  if (S_ISREG(statbuf.st_mode))
	    {
	      if (fileIsHeader(tempname))
		{
		  if (!fileInIgnoresList(tempname))
		      {
			Header *header = new Header;
			header->setDirectoryTree(this);
			header->setFileName(tempname);
			header->readFromFile();
//			header->print();
			headers->append(header);
		      }
		}
	    }
	  else
	    if (S_ISDIR(statbuf.st_mode))
	      {
		if (strcmp(direntp->d_name, ".") && strcmp(direntp->d_name, ".."))
		  {
		    char *subdir = new char[strlen(tempname) + 1];
		    strcpy(subdir, tempname);
		    subdirs.append(subdir);
		  }
	      }
	    else
	      printf("DirectoryTree::traverseDirectoryTree: %s: unknown file type\n", tempname);
	}
      else
	printf("DirectoryTree::traverseDirectoryTree: error during stat() of %s\n", tempname);

    }
  closedir(dirp);

  if (subdirs.length() > 0)
    {
      Pix placeholder;
      char *nptr;
      
      placeholder = subdirs.first();
      while(placeholder)
	{
	  nptr = subdirs(placeholder);
//	  printf("Traversing directory %s:\n", nptr);
	  traverseDirectoryTree(nptr);
	  subdirs.next(placeholder);
	}
    }
}

void
DirectoryTree::readFiles()
{
  readTypeDefs();
  readIgnores();
  readOpaques();
  readHints();
  readBases();
  if (directoryInIgnoresList(directory_name))
    {
      cerr << "DirectoryTree::readFiles: The directory to traverse " << endl << 
	"is set to be ignored (did you mean to do that?)" << endl;
      return;
    }
  traverseDirectoryTree(directory_name);
}

void
DirectoryTree::outputSchemeInterface()
{
  Pix p1;
  char *filename;
  char *headerfilename;
  FILE *fp, *hfp;

  p1 = headers->first();
  while (p1)
    {
      Header *header = (*headers)(p1);
      Pix p2;
      p2 = header->classes->first();
      while (p2)
	{
	  int must_regenerate = 1;
	  Class *tempclass = (*(header->classes))(p2);
	  filename = new char[strlen(tempclass->name) + strlen("SCM_.c++") + 1];
	  sprintf(filename, "SCM_%s.c++", tempclass->name);
	  headerfilename = new char[strlen(tempclass->name) + strlen("SCM_.h") + 1];
	  sprintf(headerfilename, "SCM_%s.h", tempclass->name);

	  if (incremental_mode)
	    {
	      struct stat buf, fbuf, hbuf;
	      
	      if (stat(headerfilename, &buf) == 0)
		{
		  if (stat(filename, &fbuf) == 0)
		    {
		      if (stat(header->filename, &hbuf) == 0)
			{
			  if ((hbuf.st_mtime < buf.st_mtime) &&
			      (hbuf.st_mtime < fbuf.st_mtime))
			    {
			      cerr << "Skipping outputting files " << filename << " and " << headerfilename << endl;
			      must_regenerate = 0;
			    }
			}
		      else
			{
			  cerr << "Error during incremental mode test of header file \"" << header->filename << "\"" << endl;
			}
		    }
		  else
		    {
		      cerr << "Error during incremental mode check of file \"" << filename << "\"" << endl;
		      perror("Error was");
		    }
		}
	      else
		{
		  cerr << "Error during incremental mode check of file \"" << headerfilename << "\"" << endl;
		  perror("Error was");
		}
	    }

	  if (must_regenerate)
	    {
	      fp = fopen(filename, "w");
	      if (fp == NULL)
		{
		  cerr << "Error opening file \"" << filename << "\" for output, aborting..." << endl;
		  exit(1);
		}
	      outputBanner(fp);
	      hfp = fopen(headerfilename, "w");
	      if (hfp == NULL)
		{
		  cerr << "Error opening file \"" << headerfilename << "\" for output, aborting..." << endl;
		  exit(1);
		}
	      outputBanner(hfp);
	      tempclass->outputHeaderFile(hfp);
	      outputIncludes(tempclass, fp);  /* we do this here because we require
						 access to all the header files to
						 resolve dependencies of where
						 data types are located. */
	      tempclass->outputSchemeInterface(fp);
	      fclose(fp);
	      fclose(hfp);
	    }

	  delete[] filename;
	  delete[] headerfilename;
	  header->classes->next(p2);
	}
      headers->next(p1);
    }
  
  if (!incremental_mode)
    {
      // output package header file and initialization file,
      // if not in incremental mode

      filename = new char[strlen(package_name) + strlen(".c++") + 1];
      sprintf(filename, "%s.c++", package_name);
      fp = fopen(filename, "w");
      if (fp == NULL)
	{
	  cerr << "Error opening file \"" << filename << "\" for output, aborting..." << endl;
	  exit(1);
	}
      headerfilename = new char[strlen(package_name) + strlen(".h") + 1];
      sprintf(headerfilename, "%s.h", package_name);
      hfp = fopen(headerfilename, "w");
      if (hfp == NULL)
	{
	  cerr << "Error opening file \"" << headerfilename << "\" for output, aborting..." << endl;
	  exit(1);
	}
      outputPackageHeaderFile(hfp);
      outputPackageInitFile(fp);
      fclose(hfp);
      fclose(fp);
      delete[] headerfilename;
      delete[] filename;

      // output makefile for package

      fp = fopen("Makefile", "w");
      if (fp == NULL)
	{
	  cerr << "Error opening file \"Makefile\" for output, aborting..." << endl;
	  exit(1);
	}
      outputPackageMakefile(fp);
      fclose(fp);
    }
}

int
DirectoryTree::isBasicDataType(char *type)
{
  return ((strcmp(type, "int") == 0) ||
	  (strcmp(type, "float") == 0) ||
	  (strcmp(type, "double") == 0) ||
	  (strcmp(type, "char") == 0) ||
	  (strcmp(type, "~") == 0));
// 	  (strcmp(type, "void") == 0) ||

}

TypeDef *
DirectoryTree::typeInTypesList(char *type)
{
  Pix p1;
  TypeDef *td;
  
  if (typedefs)
    {
      p1 = typedefs->first();
      while (p1)
	{
	  td = (*typedefs)(p1);
	  if (!strcmp(td->initial_name, type))
	    return td;
	  typedefs->next(p1);
	}
      return NULL;
    }
  return NULL;
}

int
DirectoryTree::isScopedDataType(char *type)
{
  // checks to see if a type contains
  // the scope operator (::)

  int counter;
  int flag = 0;

  for (counter = 0; counter < strlen(type); counter++)
    {
      if ((type[counter] == ':') && (flag == 1))
	{
	  return 1;
	}
      else
	{
	  if (type[counter] == ':')
	    flag = 1;
	  else
	    flag = 0;
	}
    }
  return 0;
}

char *
DirectoryTree::getBaseTypeFromScopedType(char *type)
{
  int counter;
  int flag = 0;
  int done = 0;
  char *base_type;

  counter = 0;
  while ((counter++ < strlen(type)) && !done)
    {
      if ((type[counter] == ':') && (flag == 1))
	{
	  done = 1;
	}
      else
	{
	  if (type[counter] == ':')
	    flag = 1;
	  else
	    flag = 0;
	}
    }
  
//  cerr << "type was " << type << ", counter was " << counter << endl;

  if (!done)
    return NULL;

  base_type = new char[counter - 1];
  strncpy(base_type, type, counter - 2);
  base_type[counter - 2] = 0;
  return base_type;
}

void
DirectoryTree::conditionalAddTypeToDependenciesList(char *type,
						    SLList<char *> &dependencies_list)
{
  if (!isBasicDataType(type))
    {
      // need to find out the filename where
      // this data type is declared.
	  
      // first check the typedef list
      if (!typeInTypesList(type))
	{
	  if (!isScopedDataType(type))
	    {
	      // then we'll have to search for it.
	      // add it to the dependencies list
	      // provided it isn't already in there
	      
	      Pix p2;
	      char *temptype;
	      int addflag = 1;
	      p2 = dependencies_list.first();
	      while (p2 && addflag)
		{
		  temptype = dependencies_list(p2);
		  if (strcmp(temptype, type) == 0)
		    addflag = 0;
		  else
		    dependencies_list.next(p2);
		}
	      if (addflag == 1)
		{
		  dependencies_list.append(type);
		}
	    }
	  else
	    {
	      if (!assume_all_scoped_types_are_ints)
		{
		  // add the base type of the scoped type
		  // to the dependencies list, provided it's
		  // not already in there.
		  
		  char *base_type = getBaseTypeFromScopedType(type);
		  Pix p2;
		  char *temptype;
		  int addflag = 1;
		  p2 = dependencies_list.first();
		  while (p2 && addflag)
		    {
		      temptype = dependencies_list(p2);
		      if (strcmp(temptype, type) == 0)
			addflag = 0;
		      else
			dependencies_list.next(p2);
		    }
		  if (addflag == 1)
		    {
		      cerr << "adding base type \"" << base_type << "\" to dependencies list" << endl;
		      dependencies_list.append(base_type);
		    }
		}
	    }
	}
    }
}  

void
DirectoryTree::outputBanner(FILE *fp)
{
  time_t current_time = time(NULL);
  fprintf(fp, "/*\n");
  fprintf(fp, " * This file was autogenerated by Header2Scheme on %s", ctime(&current_time));
  fprintf(fp, " * as part of %s version %s.\n", package_name,
	  package_version);
  fprintf(fp, " * For more information, contact Kenneth B. Russell <kbrussel@media.mit.edu>.\n");
  fprintf(fp, " */\n\n");
}  

void
DirectoryTree::outputIncludes(Class *the_class, FILE *fp)
{
  SLList<char *> dependencies;
  SLList<char *> resolutions;
  char *type;
  Method *method;
  Variable *variable;
  
  // start by resolving locations where
  // class variables' types are declared

  Pix p1;
  p1 = the_class->variables->first();
  while (p1)
    {
      variable = (*(the_class->variables))(p1);
      type = variable->td->type;
      conditionalAddTypeToDependenciesList(type, dependencies);
      the_class->variables->next(p1);
    }

  // now scan down the methods list.
  // have to check the return value for each method
  // as well as the types of all the arguments.

  p1 = the_class->methods->first();
  while (p1)
    {
      method = (*(the_class->methods))(p1);
      type = method->td->type;
      conditionalAddTypeToDependenciesList(type, dependencies);
      Pix p2 = method->args->first();
      while (p2)
	{
	  Argument *arg = (*(method->args))(p2);
	  type = arg->td->type;
	  conditionalAddTypeToDependenciesList(type, dependencies);
	  method->args->next(p2);
	}
      the_class->methods->next(p1);
    }
  
  // now we have the completed list of dependencies.
  // have to resolve them to filenames

  resolveDependencies(dependencies, resolutions);
  outputStandardIncludes(the_class, fp);
  outputFormattedIncludes(resolutions, fp);
  outputSchemeIncludes(dependencies, fp);
}

char *
DirectoryTree::resolveDependency(char *type)
{
  // first check to see if this data type
  // is one of the typedefs (and therefore
  // doesn't require resolution)

  if (typeInTypesList(type))
    return NULL;

  Pix p1;
  p1 = headers->first();

  while (p1)
    {
      Header *header = (*headers)(p1);
      Pix p2 = header->classes->first();
      while (p2)
	{
	  Class *a_class = (*(header->classes))(p2);
	  if (strcmp(type, a_class->name) == 0)
	    return header->filename;
	  header->classes->next(p2);
	}
      headers->next(p1);
    }
  cerr << "DirectoryTree::resolveDependency: Unable to find ";
  cerr << "the file where class \"" << type << "\" is declared" << endl;
  return NULL; 
}


void
DirectoryTree::resolveDependencies(SLList<char *> &dependencies, SLList<char *> &resolutions)
{
  Pix p1;
  char *dep;
  char *res;

  p1 = dependencies.first();
  while (p1)
    {
      dep = dependencies(p1);
      res = resolveDependency(dep);
      if (res)
	conditionalAddStringToSLList(res, resolutions);
      dependencies.next(p1);
    }
}

void
DirectoryTree::outputStandardIncludes(Class *the_class, FILE *fp)
{
  fprintf(fp, "#include <iostream.h>\n");
  fprintf(fp, "#include <stdlib.h>\n");
  fprintf(fp, "#include \"%s.h\"\n", package_name);
  // make sure the header file gets included at least once
  fprintf(fp, "#include \"SCM_%s.h\"\n", the_class->name);
}

void
DirectoryTree::formatIncludeFilename(char *filename, char *formatted)
{
  if (strlen(filename) > strlen("/usr/include"))
    {
      if (strncmp(filename, "/usr/include/", strlen("/usr/include/")) == 0)
	{
	  // got to cut off the /usr/include and
	  // make "formatted" of the form <include_filename.h>

	  formatted[0] = '<';
	  strcpy(&formatted[1], &filename[strlen("/usr/include/")]);
	  int temp = strlen(formatted);
	  formatted[temp] = '>';
	  formatted[temp+1] = 0;
	}
      else
	{
	  formatted[0] = '"';
	  strcpy(&formatted[1], filename);
	  int temp = strlen(formatted);
	  formatted[temp] = '"';
	  formatted[temp+1] = 0;
	}
    }
  else
    {
      formatted[0] = '"';
      strcpy(&formatted[1], filename);
      int temp = strlen(formatted);
      formatted[temp] = '"';
      formatted[temp+1] = 0;
    }
}

void
DirectoryTree::outputFormattedIncludes(SLList<char *> &filenames, FILE *fp)
{
  Pix p1;
  char *fn;
  char *ffn;

  p1 = filenames.first();
  while (p1)
    {
      fn = filenames(p1);
      ffn = new char[strlen(fn) + 3];
      formatIncludeFilename(fn, ffn);
      fprintf(fp, "#include %s\n", ffn);
      delete[] ffn;
      filenames.next(p1);
   }
}

void
DirectoryTree::outputSchemeIncludes(SLList<char *> &typenames, FILE *fp)
{
  Pix p1;
  char *tn;
  
  p1 = typenames.first();
  while (p1)
    {
      tn = typenames(p1);
      fprintf(fp, "#include \"SCM_");
      fprintf(fp, "%s", tn);
      fprintf(fp, ".h\"\n");
      typenames.next(p1);
    }
}

void
DirectoryTree::outputPackageHeaderFile(FILE *fp)
{
  char *uppername = stringToUppercase(package_name);

  fprintf(fp, "#ifndef _%s_\n", uppername);
  fprintf(fp, "#define _%s_\n\n", uppername);
  fprintf(fp, "#include \"SCMGlobalDispatch.h\"\n\n");
  fprintf(fp, "// prototype for main initialization routine\n");
  fprintf(fp, "extern \"C\" {\n");
  fprintf(fp, "void init_%s();\n", package_name);
  fprintf(fp, "}\n\n");
  fprintf(fp, "#endif\n");
}

void
DirectoryTree::buildGlobalClassList()
{
  // add all classes to the global class list
  
  Pix placeholder;
  Header *header;
  placeholder = headers->first();
  
  while (placeholder)
    {
      header = (*headers)(placeholder);
      Pix pl1;
      Class *cl;
      
      pl1 = header->classes->first();
      while (pl1)
	{
	  cl = (*(header->classes))(pl1);
	  global_class_list->append(cl);
	  header->classes->next(pl1);
	}
      headers->next(placeholder);
    }
}

void
DirectoryTree::reconstructClassHierarchy()
{
  int added_one = 1;
  int add_as_base_class;
  Class *front, *ptr;
  
  while ((added_one == 1) && (global_class_list->length() > 0))
    {
      added_one = 0;
      
      front = global_class_list->remove_front();
      if (front)
	{
	  add_as_base_class = classInBasesList(front->name);
	  if (classcontroller->addClassToHierarchy(SLListToCharArray(front->superclasses),
						   front->name,
						   add_as_base_class))
	    {
	      added_one = 1;
	      cerr << "Added class \"" << front->name << "\" to hierarchy" << endl;
	    }
	  else
	    {
	      global_class_list->append(front);
	      ptr = global_class_list->remove_front();
	      while (ptr != front)
		{
		  add_as_base_class = classInBasesList(ptr->name);
		  if (classcontroller->addClassToHierarchy(SLListToCharArray(ptr->superclasses),
							   ptr->name,
							   add_as_base_class))
		    {
		      added_one = 1;
		      cerr << "Added class \"" << ptr->name << "\" to hierarchy" << endl;
		      ptr = global_class_list->remove_front();
		    }
		  else
		    {
		      global_class_list->append(ptr);
		      ptr = global_class_list->remove_front();
		    }
		}
	      global_class_list->append(ptr);
	    }
	}
    }
  
  if (global_class_list->length() > 0)
    {
      Pix plgcl;
      
      cerr << "Unable to add the following classes to the hierarchy:" << endl;
      plgcl = global_class_list->first();
      while (plgcl)
	{
	  ptr = (*global_class_list)(plgcl);
	  cerr << ptr->name << endl;
	  global_class_list->next(plgcl);
	}
    }
}

void
DirectoryTree::outputPackageInitFile(FILE *fp)
{
  fprintf(fp, "#include \"%s.h\"\n\n", package_name);
  outputInitializationProcedure(fp);
}

void
DirectoryTree::outputInitializationProcedure(FILE *fp)
{
  // first figure out which order the classes have to come in
  buildGlobalClassList();
  reconstructClassHierarchy();

  Pix p1;
  ClassNode *c1;

  // output required #includes so we know about these initialization functions

  p1 = classcontroller->nodes->first();
  while (p1)
    {
      c1 = (*(classcontroller->nodes))(p1);
      fprintf(fp, "#include \"SCM_%s.h\"\n", c1->class_name);
      classcontroller->nodes->next(p1);
    }

  fprintf(fp, "void\ninit_%s()\n", package_name);
  fprintf(fp, "{\n");
  
  // now output them in the initialization procedure in that order

  p1 = classcontroller->nodes->first();
  while (p1)
    {
      c1 = (*(classcontroller->nodes))(p1);
      fprintf(fp, "init_SCM_%s();\n", c1->class_name);
      classcontroller->nodes->next(p1);
    }
  char *lower_name = stringToLowercase(package_name);
  fprintf(fp, "add_feature(\"%s\");\n", lower_name);
  delete[] lower_name;
  fprintf(fp, "classController->buildSortedClassList();\n");
  fprintf(fp, "}\n\n");
}

void
DirectoryTree::outputPackageMakefile(FILE *fp)
{
  fprintf(fp, "C++ = CC\n");
  fprintf(fp, "LOCALINCLUDES = -I. -I../SCM -I../LinkedList -I%s\n", directory_name);
  fprintf(fp, "# o32 opts\n");
  fprintf(fp, "SGIFLAGS = -o32 -woff 3209 -MDupdate Makedepend\n");
  fprintf(fp, "# n32 opts\n");
  fprintf(fp, "#SGIFLAGS = -n32 -woff 1116 -MDupdate Makedepend\n");
  fprintf(fp, "C++FLAGS= -g $(SGIFLAGS) $(LOCALINCLUDES) -DHAVE_CONFIG_H -DUSE_ANSI_PROTOTYPES -DFLOATS -DBIGSMOBS -DALLOCATEZERO\n");

  fprintf(fp, "SRCS = \\\n");

  // output the source files which will be generated
  Pix p1;
  ClassNode *c1;
  p1 = classcontroller->nodes->first();
  while (p1)
    {
      c1 = (*(classcontroller->nodes))(p1);
      fprintf(fp, "\tSCM_%s.c++\\\n", c1->class_name);
      classcontroller->nodes->next(p1);
    }
  fprintf(fp, "\t%s.c++\n", package_name);
  
  fprintf(fp, "\n");

  fprintf(fp, "OBJS = $(SRCS:.c++=.o)\n");
  fprintf(fp, "TARGETS=lib%s.a\n", package_name);
  fprintf(fp, "all: $(TARGETS)\n\n");
  fprintf(fp, "clean:\n");
  fprintf(fp, "\trm $(OBJS) $(TARGETS)\n\n");
  fprintf(fp, "depend:\n");
  fprintf(fp, "\tmakedepend -I/usr/include/CC -- $(C++FLAGS) -- $(SRCS)\n\n");
  fprintf(fp, "lib%s.a: $(OBJS)\n", package_name);
  fprintf(fp, "\tar crs $@ $(OBJS)\n");
  fprintf(fp, "#\tCC -ar -o $@ $(OBJS)\n");
  fprintf(fp, "#\tranlib $@\n\n");
  fprintf(fp, "lib%s.so: $(OBJS)\n", package_name);
  fprintf(fp, "\tCC $(C++FLAGS) -shared -o $@ $(OBJS)\n\n");
  fprintf(fp, ".SUFFIXES: .c++\n\n");
  fprintf(fp, ".c++.o:\n");
  fprintf(fp, "\t$(C++) $(C++FLAGS) -c $*.c++\n\n");
  fprintf(fp, "# DO NOT DELETE THIS LINE -- make depend depends on it.\n");
}
