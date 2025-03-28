############################
# FLAGS:                   #
############################

# tell automake to pass certain flags to autoconf on re-run
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

# tell automake to pass certain flags to 'ctags'
AM_CTAGSFLAGS = --extra=+f

#FIXME: don't assume GCC, do feature detection in configure.ac
# tell automake to pass certain CFLAGS to the compiler
AM_CFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/include/liblp \
	-pedantic-errors \
	-Wpedantic \
	-Wall \
	-Wextra \
	-Wdouble-promotion \
	-Wswitch-default

# initialize variables
EXTRA_DIST =
CLEANFILES =
BUILT_SOURCES = 

############################
# LIBRARIES TO BUILD:	   #
############################

# tell automake which libraries to build
lib_LTLIBRARIES = liblp.la

#----------------------------#
# liblp.la build options #
#----------------------------#

# list the names of the public header files of the liblp.la library
liblp_la_include_HEADERS = \
	include/liblp/atom.h \
	include/liblp/io.h \
	include/liblp/rule.h \
	include/liblp/symbol.h \
	include/liblp/version.h

# list all source code files for the liblp.la library
liblp_la_SOURCES = \
	src/atom.c \
	src/input.c \
	src/output.c \
	src/rule.c \
	src/symbol.c \
	src/version.c

# tell automake that the installation directory for public header files of the
# liblp.la library should be a subfolder in the default ${includedir}
liblp_la_includedir = ${includedir}/liblp

# tell libtool to pass certain LDFLAGS to the compiler when building libsharp.la
#
# 1) Specify the Shared Object Version Information for the library, which
# consists of three numbers: CURRENT:REVISION:AGE
#
#   CURRENT: This is the version of the (external) ABI in the library.
#       Increase if the ABI changes (i.e. interfaces added or changed)!
#
#   REVISION: The revision represents how often the library was changed for a
#       given ABI version. Increase this whenever the library code changes, but
#       reset to 0 when incrementing CURRENT.
#
#   AGE: This is the difference between the oldest and newest ABI version
#       that the library is backwards compatible with (e.g. if CURRENT is 4,
#       but we are backwards compatible to ABI version 1, then AGE = 3). Set to
#       0 if there is a backwards-compatibility breaking change, otherwise
#       increase by 1 if CURRENT was increased by one.
#
liblp_la_LDFLAGS = -version-info 0:0:0

############################
# PROGRAMS TO BUILD:	   #
############################

# Link the main library (liblp.la) to all programs.
LDADD = liblp.la

# tell automake which programs to build
bin_PROGRAMS = len lplist lpstrip

#-----------------------#
# program build options #
#-----------------------#

# list all source code files for the programs
len_SOURCES = src/utils/len.c
lplist_SOURCES = src/utils/lplist.c
lpstrip_SOURCES = src/utils/lpstrip.c

############################
# DISTRIBUTION OPTIONS:	   #
############################

# tell automake about scripts that should be distributed in the tarball, but
# not installed on "make install"
dist_noinst_SCRIPTS = autogen.sh

# tell automake to clean the generated files on "make clean"
CLEANFILES += $(BUILT_SOURCES)

############################
# UNIT TESTS:              #
############################

# tell automake to recursively go into the test directory
#SUBDIRS = tests
