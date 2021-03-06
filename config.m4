dnl $Id$
dnl config.m4 for extension bmvc

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(bmvc, for bmvc support,
dnl Make sure that the comment is aligned:
dnl [  --with-bmvc             Include bmvc support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(bmvc, whether to enable bmvc support,
dnl Make sure that the comment is aligned:
[  --enable-bmvc           Enable bmvc support])

if test "$PHP_BMVC" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-bmvc -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/bmvc.h"  # you most likely want to change this
  dnl if test -r $PHP_BMVC/$SEARCH_FOR; then # path given as parameter
  dnl   BMVC_DIR=$PHP_BMVC
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for bmvc files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       BMVC_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$BMVC_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the bmvc distribution])
  dnl fi

  dnl # --with-bmvc -> add include path
  dnl PHP_ADD_INCLUDE($BMVC_DIR/include)

  dnl # --with-bmvc -> check for lib and symbol presence
  dnl LIBNAME=bmvc # you may want to change this
  dnl LIBSYMBOL=bmvc # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $BMVC_DIR/lib, BMVC_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_BMVCLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong bmvc lib version or lib not found])
  dnl ],[
  dnl   -L$BMVC_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(BMVC_SHARED_LIBADD)

  PHP_NEW_EXTENSION(bmvc, bmvc.c, $ext_shared)
fi
