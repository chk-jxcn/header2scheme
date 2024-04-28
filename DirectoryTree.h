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

#ifndef _DIRECTORY_TREE_
#define _DIRECTORY_TREE_

#include "SLList.h"
#include "Utilities.h"
#include "Header.h"
#include "ClassController.h"

class TypeDef {
public:
  char *initial_name;
  TypeDesc *resolved_type;
  
  TypeDef() { initial_name = NULL; resolved_type = new TypeDesc; }
  ~TypeDef() { if (initial_name) delete[] initial_name; delete resolved_type; }
};

class Ignore {
public:
  enum IgnoreType {
    DIRECTORY,
    FILE,
    CLASS,
    METHOD
    };   // more to come...
  IgnoreType type;
  union {
    char *directory_name;
    char *file_name;
    char *class_name;
    struct {
      char *method_name;
      char *method_type;
      char *class_name;
    } method_ignore;
    } data;
};

class Opaque {
public:
  char *type_name;
  char *path_name;  // pathname of file where this type is defined in C++
  int exists_as_pointer_only;   /* set this to 1 to only output conversion procedure from pointer
				   (i.e. not from reference).
				   This should be used for void *'s, function pointers, etc. */
  Opaque()  { type_name = NULL; path_name = NULL; exists_as_pointer_only = 0; }
  ~Opaque() {}
};

class Hint {
public:
  TypeDesc *original_typedesc;
  int user_choice;
};

class DirectoryTree {
 public:
  DirectoryTree(char *user_directory_name,
		char *user_package_name,
		char *user_package_version,
		char *user_ignores_filename = NULL,
		char *user_types_filename = NULL,
		char *user_opaques_filename = NULL,
		char *user_hints_filename = NULL,
		char *user_bases_filename = NULL,
		char *user_macros_filename = NULL,
		int user_incremental_mode = 0,
		int user_assume_all_scoped_types_are_ints = 0);
  ~DirectoryTree();

  void readFiles();
  void outputSchemeInterface();
  
  SLList<Header *> *headers;
  SLList<Class *> *global_class_list;

  friend class Class;
  friend class Header;
 protected:
  char *directory_name;
  char *package_name;
  char *package_version;
  char *ignores_filename;
  char *types_filename;
  char *opaques_filename;
  char *hints_filename;
  char *bases_filename;
  char *macros_filename;

  ClassController *classcontroller;

  SLList<TypeDef *> *typedefs;
  SLList<Ignore *> *ignores;
  SLList<Opaque *> *opaques;
  SLList<Hint *> *hints;
  SLList<char *> *bases;

  const int COMMAND_LENGTH;
  char *command_buf;

  int assume_all_scoped_types_are_ints;
  int incremental_mode;

  void readTypeDefs();
  void readIgnores();
  void readOpaques();
  void readHints();
  void readBases();

  int fileIsHeader(char *filename);
  int directoryInIgnoresList(char *dirname);
  int fileInIgnoresList(char *filename);
  int classInIgnoresList(char *class_name);
  int methodInIgnoresList(char *class_name, char *method_name,
			  char *method_type);

  void traverseDirectoryTree(char *pathname);

  void outputBanner(FILE *fp);

  int isBasicDataType(char *type);
  TypeDef *typeInTypesList(char *type);
  void outputIncludes(Class *the_class, FILE *fp);

  int typeDescInHintsList(TypeDesc *td);

  int classInBasesList(char *class_name);

  int usingMacros();
  char *getMacroCommand(const char *filename);

  void outputFormattedIncludes(SLList<char *> &filenames, FILE *fp);
  void outputSchemeIncludes(SLList<char *> &typenames, FILE *fp);
  void formatIncludeFilename(char *filename, char *formatted);
  
  void outputPackageHeaderFile(FILE *fp);
  void buildGlobalClassList();
  void reconstructClassHierarchy();
  void outputPackageInitFile(FILE *fp);
  void outputInitializationProcedure(FILE *fp);
  void outputPackageMakefile(FILE *fp);

  int isScopedDataType(char *type);
  char *getBaseTypeFromScopedType(char *type);
  void conditionalAddTypeToDependenciesList(char *type, SLList<char *> &dependencies_list);
  char *resolveDependency(char *type);
  void resolveDependencies(SLList<char *> &dependencies, SLList<char *> &resolutions);
  void outputStandardIncludes(Class *the_class, FILE *fp);
};

#endif
