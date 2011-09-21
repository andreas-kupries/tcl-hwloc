# tclhwloc.m4 --
#
#	This file provides a set of autoconf macros internal and
#	custom to the tcl-hwloc package.
#
# Copyright (c) 2011	Andreas Kupries
#
# See the file "license.terms" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.

# RCS: @(#) $Id: tcl.m4,v 1.157 2010/12/15 05:35:07 stwo Exp $

AC_PREREQ(2.57)

#------------------------------------------------------------------------
# TCLHWLOC_LOCATE_HWLOC --
#
# Arguments:
#	Minimum required version of hwloc.
#
# Requires:
# Results:
#	Adds the --with-hwloc switch to configure
#
#	Defines and substs the following variables:
#		HWLOC_CFLAGS
#		HWLOC_LIBS
#
#------------------------------------------------------------------------

AC_DEFUN([TCLHWLOC_LOCATE_HWLOC],[
    AC_MSG_CHECKING([for hwloc])

    AC_ARG_WITH([hwloc],
	AC_HELP_STRING([--with-hwloc],
		       [Provide non-standard location of hwloc]),
	[with_uc_hwloc=${withval}])

    # The search path for hwloc's package configuration, and thus
    # location, is communicated through the environment variable
    # PKG_CONFIG_PATH. pkg-config unfortunately doesn't seem to
    # have cmdline flags for that. Existing settings of PKG_CONFIG_PATH
    # are preserved and passed.

    if test x"[$]{with_uc_hwloc}" == x ; then
        # Use default location, derived from prefix, exec_prefix.
	PKG_CONFIG_PATH="[$]{prefix}/lib/pkgconfig:[$]{exec_prefix}/lib/pkgconfig:[$]{PKG_CONFIG_PATH}"
    else
        # Use locations derived from --with-hwloc
	PKG_CONFIG_PATH="[$]{with_uc_hwloc}:[$]{with_uc_hwloc}/pkgconfig:[$]{PKG_CONFIG_PATH}"
    fi
    export PKG_CONFIG_PATH

    # First check, does it exist at all ?
    if pkg-config --exists hwloc
    then
	:
    else
	AC_MSG_ERROR([Could not find hwloc. Please use --with-hwloc to provide its location.])
    fi

    # Second check, is it the correct version ?
    if pkg-config --atleast-version=$1 hwloc
    then
	:
    else
	AC_MSG_ERROR([Hwloc version >= $1 required, not found. Please use --with-hwloc to provide the location of a proper hwloc.])
    fi

    HWLOC_CFLAGS=`pkg-config --cflags hwloc`
    HWLOC_LIBS=`pkg-config --libs hwloc`

    AC_MSG_RESULT([ok, cflags .... $HWLOC_CFLAGS]) 
    AC_MSG_RESULT([........................, ldflags ... $HWLOC_LIBS]) 

    TEA_ADD_LIBS([$HWLOC_LIBS])
    TEA_ADD_CFLAGS([$HWLOC_CFLAGS])
])

# Local Variables:
# mode: autoconf
# End:
