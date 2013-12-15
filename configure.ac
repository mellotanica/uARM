#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([uARM], [0.1], [melletti.marco@gmail.com])
AC_CONFIG_SRCDIR([services/debugger.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lQtCore':
AC_CHECK_LIB([QtCore], [main])
# FIXME: Replace `main' with a function in `-lQtGui':
AC_CHECK_LIB([QtGui], [main])
# FIXME: Replace `main' with a function in `-lelf':
AC_CHECK_LIB([elf], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([memset strerror strtol strtoul])

#AC_CONFIG_FILES([Makefile])
AC_OUTPUT
