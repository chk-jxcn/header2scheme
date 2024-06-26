# Makefile for scm4e1 (Scheme implementation intended for JACAL).
# Copyright (C) 1990, 1991, 1992, 1993, 1994 Aubrey Jaffer.
# See the file "COPYING" for terms applying to this program

# directory where COPYING and Init.scm reside.
#IMPLPATH=/usr/local/src/scm/
#this one is good while debugging
IMPLPATH=`pwd`/

# Pathname where Init.scm resides.  This directory must also contain COPYING.
IMPLINIT=$(IMPLPATH)Init.scm
# If pathname where Init.scm resides is not known in advance then
# SCM_INIT_PATH is the environment variable whose value is the
# pathname where Init.scm resides.
# IMPLINIT=

# directory where `make install' will put executable.
DEST=/usr/local/bin/
# directory where `make install' will put manual page.
MANDEST=/usr/local/man/man1/

#CC = your compiler
# -DRTL if this is a run-time library only (no interactive top level)
# -Dunix is required for SCO
# -DSVR4 is required for Solaris 2.x and other SVR4 clones
# -static is required with gcc under OSF/1 V1.2 on DEC Alpha.
# -DRECKLESS if you want most scm error checking disabled.
# -DSTACK_LIMIT=(HEAP_SEG_SIZE/2) to enable stack checking.
# -O if you want the optimizing C compiler to be used.
# -g for dynamic linking (symbols).
#CFLAGS=+O1		#workaround for HP-UX 9.01 bug.
#CFLAGS=-Wl,-E		#dynamic link flags for HP-UX 9.01.
#CFLAGS=-O
#CC = cc -hvector2 -hscalar2		#for UNICOS, inlining breaks gc.

LOCALINCLUDES = -I.  \
		-I../LinkedList

LOCALLIBS = 	-L.  \
		-L../LinkedList/lib/$(MACHTYPE) \
		-lSLList -lInventor

# o32 opts
SGIFLAGS = -o32
# n32 opts
#SGIFLAGS = -n32
CFLAGS = -g -cckr $(SGIFLAGS) -D__STDC__ $(LOCALINCLUDES)
C++FLAGS = -g $(SGIFLAGS) -D__STDC__ $(LOCALINCLUDES)
CC = cc
C++ = CC
#CC = gcc -O2 -traditional
LD = ld

#FFLAGS are flags for inexact numbers (floating point)
# -DFLOATS turns on support for inexact numbers.
# -DENGNOT for engineering notation instead of scientific notation.

FFLAGS=-DFLOATS -DSUN_DL

# append any names of user extension files
# -lansi for SCO and XENIX
# -lm for -DFLOATS
# -lcurses for `curses'; some systems also require -ltermcap
# -lncurses on linux (curses has bugs).
# ELIBS=../regex-0.11/regex.o
LIBS=$(ELIBS) -lm

# RGXFLAGS=-I../regex-0.11/

# INITS= the initialization calls for user extension files.
#File:	INITS line:		functions defined:
#sc2.c	init_sc2\(\)	substring-move-left!, substring-move-right!,
#			substring-fill!, append!, and last-pair
#rgx.c	init_rgx\(\)	regcomp and regexec.
#crs.c	init_crs\(\)	lots.

INITS=$(DFLAG) -DINITS=\;

#XFILES = user extension files or machine specific files.
#XFILES = rgx.o dynl.o
#XFILES = setjump.o #for Cray YMP 
#XFILES = sc2.o

XFILES= SCMGlobalDispatch.o ClassController.o StringList.o Scheme.o hash.o dynl.o number-test.o

default: libscm.a

athena:
	make -f Makefile.athena libscm.a

athenascm:
	make -f Makefile.athena scm

SCMGlobalDispatch.o: SCMGlobalDispatch.c++
	$(C++) $(C++FLAGS) $(LOCALINCLUDES) -c $(FFLAGS) $(DFLAG) $(INITS) SCMGlobalDispatch.c++

ClassController.o: ClassController.c++
	$(C++) $(C++FLAGS) $(LOCALINCLUDES) -c $(FFLAGS) $(DFLAG) $(INITS) ClassController.c++

StringList.o: StringList.c++
	$(C++) $(C++FLAGS) $(LOCALINCLUDES) -c $(FFLAGS) $(DFLAG) $(INITS) StringList.c++

Scheme.o: Scheme.c++
	$(C++) $(C++FLAGS) $(LOCALINCLUDES) -c $(FFLAGS) $(DFLAG) $(INITS) Scheme.c++

hash.o: hash.c++
	$(C++) $(C++FLAGS) $(LOCALINCLUDES) -c $(FFLAGS) $(DFLAG) $(INITS) hash.c++

number-test.o: number-test.c
	$(CC) $(CFLAGS) $(LOCALINCLUDES) -c $(FFLAGS) $(DFLAG) $(INITS) number-test.c

#you should not need to change below this line.

DFLAG = -DIMPLINIT=\"$(IMPLINIT)\"
ffiles = time.o repl.o fscl.o sys.o feval.o subr.o funif.o $(XFILES)
efiles = time.o repl.o escl.o sys.o eeval.o subr.o eunif.o $(XFILES)

SHELL = /bin/sh
intro:
	@echo
	@echo "This is scm$(VERSION).  Read \"README\" to learn how to"
	@echo "build and install SCM.  Here is a quick guide."
	@echo
	@echo "  From: bos@scrg.cs.tcd.ie"
	@echo "  New build and install scripts using GNU autoconf are"
	@echo "  available as scmconfig.tar.gz in the SCM distribution"
	@echo "  directories.  See README.unix in scmconfig.tar.gz for"
	@echo "  further instructions."
	@echo
	@echo "  Alternatively:"
	@echo "      cp makefile.unix Makefile"
	@echo "  Edit Makefile to change CFLAGS, LIBS, and especially"
	@echo "  IMPLINIT.  If you are compiling under HP-UX, see"
	@echo "  comments in Makefile (fscm.o)."
	@echo "      make scm"
	@echo
	@echo "  Once you have built scm successfully, test it:"
	@echo "      make check"
	@echo

all:	scm
scheme:	scm

# -DINITS= the initialization calls for user extension files.
dbscm:	escm.a ../wb/db.a scm.c scm.h scmfig.h patchlvl.h makefile.unix
	$(CC) -o dbscm $(ELIBS) \
	$(CFLAGS) $(DFLAG) $(INITS)init_db\(\)\; \
	scm.c escm.a ../wb/db.a
	rm escm.a
curscm:	escm.a rgx.o crs.o scm.c
	$(CC) -o curscm $(ELIBS) \
	$(CFLAGS) $(DFLAG) $(INITS)init_crs\(\)\;init_rgx\(\) \
	scm.c escm.a rgx.o crs.o -lncurses
	rm escm.a
escm.a:	$(efiles) makefile.unix
	ar crv escm.a $(efiles)

dscm:	$(ffiles) dscm.o dynl.o
#	$(CC) -o dscm $(ffiles) dscm.o dynl.o $(LIBS) -ldl
		# for Sun-OS 4.1.x (and maybe Solaris)
	$(CC) -o dscm $(ffiles) dscm.o dynl.o $(LIBS) -ldld
	strip -S -X -x dscm
		# for Linux
shlscm:	$(ffiles) dscm.o dynl.o
		# -WlE for HP-UX
	$(CC) -Wl,-E -o dscm $(ffiles) dscm.o $(LIBS) dynl.o -ldld
dscm.o:	scm.c scm.h scmfig.h patchlvl.h
	$(CC) $(CFLAGS) -c $(FFLAGS) $(DFLAG) \
	-DINITS=init_dynl\(\) scm.c
	mv scm.o dscm.o

scm:	$(ffiles) fscm.o
	$(C++) -o scm $(ffiles) fscm.o $(LOCALLIBS) $(LIBS)
fscm.o:	scm.c++ scm.h scmfig.h patchlvl.h
	$(C++) $(CFLAGS) -c $(FFLAGS) $(DFLAG) $(INITS) scm.c++
	mv scm.o fscm.o
#From: fred@sce.carleton.ca (Fred J Kaudel)
#-U, if get can't include machine file errors from sys/signal.h during
#cc [-Ac] compiles under HP-UX A.08.07, or similar, then try changing to
#either cc -Aa, if available, for this file only, or replace the above $(CC)
#line with the following:
#	$(CC) $(CFLAGS) -c $(FFLAGS) $(DFLAG) $(INITS) scm.c \
#		 -U__hp9000s800

fscl.o:	scl.c scm.h scmfig.h
	$(CC) $(CFLAGS) -c $(FFLAGS) scl.c
	mv scl.o fscl.o
feval.o:	eval.c scm.h scmfig.h setjump.h
	$(CC) $(CFLAGS) -c $(FFLAGS) eval.c
	mv eval.o feval.o
funif.o:	unif.c scm.h
	$(CC) $(CFLAGS) -c $(FFLAGS) unif.c
	mv unif.o funif.o

escm:	$(efiles) escm.o
	$(CC) -o escm $(efiles) escm.o $(ELIBS)
escm.o:	scm.c scm.h scmfig.h patchlvl.h
	$(CC) $(CFLAGS) -c $(DFLAG) $(INITS) scm.c
	mv scm.o escm.o
escl.o:	scl.c scm.h scmfig.h
	$(CC) $(CFLAGS) -c scl.c
	mv scl.o escl.o
eeval.o:	eval.c scm.h scmfig.h setjump.h
	$(CC) $(CFLAGS) -c eval.c
	mv eval.o eeval.o
eunif.o:	unif.c scm.h
	$(CC) $(CFLAGS) -c unif.c
	mv unif.o eunif.o

repl.o:	repl.c scm.h scmfig.h setjump.h
	$(CC) $(CFLAGS) -c repl.c
sys.o:	sys.c scm.h scmfig.h setjump.h
	$(CC) $(CFLAGS) -c sys.c
time.o:	time.c scm.h scmfig.h
	$(CC) $(CFLAGS) -c time.c
subr.o:	subr.c scm.h scmfig.h
	$(CC) $(CFLAGS) -c subr.c
dynl.o:	dynl.c scm.h scmfig.h
	$(CC) $(CFLAGS) -DSUN_DL -DSVR4 -c dynl.c	# -DRTL
#	$(CC) $(CFLAGS) -DDLD -c dynl.c
		# -DRTL

# Now for dynamically loadable files:

sc2.o:	sc2.c scm.h
	$(CC) $(CFLAGS) -c sc2.c
rgx.o:	rgx.c scm.h scmfig.h
	$(CC) $(CFLAGS) $(RGXFLAGS) -c rgx.c
crs.o:	crs.c scm.h scmfig.h
	$(CC) $(CFLAGS) -c crs.c -I/usr/include/ncurses
				# ncurses include for linux
turtlegr.o:	turtlegr.c scm.h scmfig.h makefile.unix
	$(CC) -c $(CFLAGS) $(FFLAGS) -DX11 -I/usr/local/X11R5/include \
	turtlegr.c

#		for HP-UX
shlib:
	$(CC) $(CFLAGS) -c +z sc2.c
	$(LD) -b sc2.o -o sc2

both:	scm escm

libscm.a:	rtlscm.o $(ffiles)
	rm -f libscm.a
	ar crs libscm.a rtlscm.o $(ffiles)
#	CC -ar -o libscm.a rtlscm.o $(ffiles)
#	ar rc libscm.a rtlscm.o $(ffiles)
#	ranlib libscm.a

rtlscm.o:	scm.c++ scm.h scmfig.h patchlvl.h
	$(C++) $(C++FLAGS) -c $(FFLAGS) -DRTL $(DFLAG) \
	$(INITS)init_user_scm\(\) scm.c++ -o rtlscm.o

scm.doc:	scm.1
	nroff -man scm.1 >scm.doc

install:	scm scm.1
	-cp scm $(DEST)
	-strip $(DEST)scm
	-cp scm.1 $(MANDEST)
	-cp Init.scm $(IMPLPATH)
	-cp Transcen.scm $(IMPLPATH)
	-cp COPYING $(IMPLPATH)

proto.h:	$(cfiles)
	rm -f proto.h
	mkproto $(cfiles) > proto.h

#### Stuff for maintaining SCM below ####

VERSION = 4e1
ver = $(VERSION)
cfiles = scm.c time.c repl.c scl.c sys.c eval.c subr.c sc2.c unif.c \
	rgx.c crs.c dynl.c
confiles = INSTALL.unix configure configure.in scmconfig.h.in \
	mkinstalldirs Makefile.in acconfig-1.5.h
hfiles = scm.h scmfig.h setjump.h patchlvl.h
ifiles = Init.scm Transcen.scm Link.scm
tfiles = test.scm example.scm pi.scm pi.c split.scm
dfiles = ANNOUNCE README COPYING scm.1 scm.doc QUICKREF MANUAL ChangeLog \
	code.doc proto.h
mfiles = makefile.unix makefile.msc makefile.bor makefile.tur\
	makefile.djg makefile.emx makefile.qc compile.amiga link.amiga \
	makefile.aztec makefile.ast makefile.prj dmakefile makefile.wcc \
	makefile.acorn
vfiles = setjump.mar VMSBUILD.COM VMSGCC.COM setjump.s
afiles = $(dfiles) $(cfiles) $(hfiles) $(ifiles) $(tfiles) $(mfiles) $(vfiles)

makedev = make -f $(HOME)/makefile.dev
dest = $(HOME)/dist/
temp/scm:	$(afiles)
	-rm -rf temp
	mkdir temp
	mkdir temp/scm
	ln  $(afiles) temp/scm

dist:	$(dest)scm$(VERSION).tar.gz
$(dest)scm$(VERSION).tar.gz:	temp/scm
	$(makedev) DEST=$(dest) PROD=scm ver=$(VERSION) tar.gz
shar:	scm.shar
scm.shar:	temp/scm
	$(makedev) PROD=scm shar
dclshar:	scm.com
com:	scm.com
scm.com:	temp/scm
	$(makedev) PROD=scm com
zip:	scm.zip
scm.zip:	temp/scm
	$(makedev) PROD=scm zip
distzip:	scm$(VERSION).zip
scm$(VERSION).zip:	temp/scm turtle turtlegr.c grtest.scm
	$(makedev) DEST=$(dest) PROD=scm ver=$(VERSION) zip
	cd ..; zip -ur $(dest)scm$(VERSION).zip \
		 scm/turtle scm/turtlegr.c scm/grtest.scm
	mv $(dest)scm$(VERSION).zip /d/scm/dist/
pubzip:	temp/scm
	$(makedev) DEST=$(HOME)/pub/ PROD=scm zip

diffs:	pubdiffs
pubdiffs:	temp/scm
	$(makedev) DEST=$(HOME)/pub/ PROD=scm pubdiffs
distdiffs:	temp/scm
	$(makedev) DEST=$(dest) PROD=scm ver=$(ver) distdiffs 

new:
	chpat scm$(VERSION) scm$(ver) ANNOUNCE ../jacal/ANNOUNCE \
		../wb/README ../wb/ANNOUNCE d/scm/dist/readme.txt \
		/d/scm/dist/install.bat /d/scm/dist/makefile
	chpat $(VERSION) $(ver) README MANUAL patchlvl.h Init.scm makefile.unix

temp/scmconfig:	$(confiles)
	-rm -rf temp/scmconfig
	-mkdir temp
	mkdir temp/scmconfig
	ln  $(confiles) temp/scmconfig
confdist:	scmconfig.tar.gz
scmconfig.tar.gz:	temp/scmconfig
	cd temp; tar cohf ../scmconfig.tar scmconfig
	chmod 664 scmconfig.tar
	-rm -f scmconfig.tar.*z
	gzip scmconfig.tar
	chmod 664 scmconfig.tar.*z

checks: check echeck
check:	scm test.scm
	scm -ftest.scm -e'(test-sc4)(test-cont)(test-inexact)(gc)'
echeck:	escm test.scm
	scm -ftest.scm -e'(test-sc4)(test-cont)(gc)'

lint:	lints
lints:	$(cfiles) $(hfiles)
	lint $(CFLAGS) $(FFLAGS) $(cfiles) | tee lints
#	lint $(CFLAGS) $(cfiles) | tee lintes

#seds to help find names not unique in first 8 characters (name8s)
# for BSD nm format
SED_TO_STRIP_NM=sed -e '/.*\.o$$/d' -e 's/.* _//' -e 's/.* T //'
#old, bad for T [^_] on suns: SED_TO_STRIP_NM=sed -e '/.*\.o$$/d' -e 's/.* _//'
# For a System V nm where plain C identifiers have _ prepended:
#SED_TO_STRIP_NM=sed -e '/^_[A-Za-z][A-za-z0-9_]*[ 	|].*|extern|!d' -e 's/|/ /g' -e 's/^_//'
# For a System V nm where plain C identifiers have nothing prepended:
#SED_TO_STRIP_NM=sed -e '/^[A-Za-z][A-za-z0-9_]*[ 	|].*|extern|!d' -e 's/|/ /g'

name8:	name8s
name8s:	scm
	nm scm |\
	$(SED_TO_STRIP_NM) |\
	sort -u|\
	awk '{	if (substr(l,1,8)==substr($$1,1,8)) {\
			if (p) print l;\
			print $$1;p=0;stat=1\
		}else p=1;\
		l=$$1\
	     }END{exit stat}' -
ctags:	$(hfiles) $(cfiles) code.doc
	etags $(hfiles) $(cfiles) code.doc
TAGS:
tags:	$(hfiles) $(cfiles) $(ifiles) $(mfiles) $(vfiles) \
	MANUAL code.doc README
	etags $(hfiles) $(cfiles) $(ifiles) $(mfiles) $(vfiles) \
	MANUAL code.doc README
mostlyclean:
clean:
	-rm -f *~ *.bak *.orig *.rej core a.out lints tmp* \#* *\#
	-rm -rf *temp
distclean:
	-rm -f *~ *.bak *.orig *.rej core a.out *.o lints tmp* \#* *\#
	-rm -rf *temp
realclean:
	-rm -f *~ *.bak *.orig *.rej TAGS core a.out *.o lints tmp* \
		scm.doc \#* *\#
	-rm -rf *temp
realempty:	temp/scm
	-rm -f $(afiles)
