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

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <unistd.h>
#include "DirectoryTree.h"

#define H2S_VERSION "1.3"

void
usage()
{
  cerr << "Usage: Header2Scheme -d directory_name -p package_name -v version_number" << endl;
  cerr << "    [-t typedef_file_name] [-i ignores_file_name] [-o opaques_file_name]" << endl;
  cerr << "    [-h hints_file_name] [-b bases_file_name] [-m macros_file_name]" << endl;
  cerr << "    [-n]" << endl;
  cerr << "Note: -n turns on incremental mode." << endl;
}

void
banner()
{
  cerr << "This is Header2Scheme version " << H2S_VERSION << ", " <<
    "Copyright (C) 1995 Kenneth B. Russell." << endl;
  cerr << "Header2Scheme comes with ABSOLUTELY NO WARRANTY; " << endl;
  cerr << "see the file \"LICENSE\" for details." << endl;
  cerr << "This is free software, and you are welcome to redistribute it " << endl;
  cerr << "under certain conditions; see the file \"LICENSE\" for details." << endl;
}
  

int
main(int argc, char **argv)
{
  int c;
  char *directory_name = NULL;
  char *package_name = NULL;
  char *package_version = NULL;
  char *typedef_name = NULL;
  char *ignores_name = NULL;
  char *opaques_name = NULL;
  char *hints_name = NULL;
  char *bases_name = NULL;
  char *macros_name = NULL;
  int incremental_mode = 0;
  
  banner();

  while ((c = getopt(argc, argv, "nd:p:v:t:i:o:h:b:m:")) != EOF)
    {
      switch (c)
	{
	case 'n':
	  {
	    incremental_mode = 1;
	    break;
	  }

	case 'd':
	  {
	    directory_name = optarg;
	    break;
	  }
	  
	case 'p':
	  {
	    package_name = optarg;
	    break;
	  }

	case 'v':
	  {
	    package_version = optarg;
	    break;
	  }

	case 't':
	  {
	    typedef_name = optarg;
	    break;
	  }

	case 'i':
	  {
	    ignores_name = optarg;
	    break;
	  }

	case 'o':
	  {
	    opaques_name = optarg;
	    break;
	  }

	case 'h':
	  {
	    hints_name = optarg;
	    break;
	  }

	case 'b':
	  {
	    bases_name = optarg;
	    break;
	  }

	case 'm':
	  {
	    macros_name = optarg;
	    break;
	  }

	default:
	  {
	    cerr << "Invalid option: " << (char) c << endl;
	    usage();
	    exit(1);
	  }
	}
    }

  if ((directory_name == NULL) || (package_name == NULL) || (package_version == NULL))
    {
      usage();
      exit(1);
    }

  DirectoryTree *dt = new DirectoryTree(directory_name,
					package_name,
					package_version,
					ignores_name,
					typedef_name,
					opaques_name,
					hints_name,
					bases_name,
					macros_name,
					incremental_mode);
  dt->readFiles();
  dt->outputSchemeInterface();
  return 0;
}
