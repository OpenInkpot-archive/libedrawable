AC_PREREQ(2.57)
AC_INIT(edrawable, 0.2, avn@daemon.hole.ru)
AM_INIT_AUTOMAKE(foreign)

AC_LANG_C
AC_PROG_CC
AM_PROG_LIBTOOL

PKG_CHECK_MODULES(FREETYPE,[freetype2])
PKG_CHECK_MODULES(EVAS,[evas])
PKG_CHECK_MODULES(ECORE,[ecore])
PKG_CHECK_MODULES(ECORE_FILE,[ecore-file])

AC_OUTPUT(Makefile edrawable.pc src/Makefile)
