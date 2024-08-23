dnl Initialise Autoconf
AC_PREREQ([2.69])
AC_INIT(
	[nestopia],
	[1.52.1])
AC_CONFIG_SRCDIR([source])
AC_LANG([C++])



dnl ========================================================================
dnl Check whether we want to set defaults for CXXFLAGS, CPPFLAGS and LDFLAGS
dnl ========================================================================
AC_MSG_CHECKING([whether configure should try to set CXXFLAGS/CPPFLAGS/LDFLAGS])
AS_IF([test "x${CXXFLAGS+set}" = "xset" || test "x${CPPFLAGS+set}" = "xset" || test "x${LDFLAGS+set}" = "xset"],
	[enable_flags_setting=no],
	[enable_flags_setting=yes]
)
AC_MSG_RESULT([${enable_flags_setting}])

AX_CHECK_ENABLE_DEBUG
AS_IF([test "x${enable_debug}" = "xno"],
	[
		cxxflags_test="-O3"
		cppflags_test=""

		AC_CANONICAL_HOST
		AS_CASE([${host_os}],
			[darwin*],
			[ldflags_test="-Wl,-dead_strip_dylibs"],
			[ldflags_test="-Wl,-O1 -Wl,--as-needed"]
		)
	]
)



dnl =======================================
dnl Check for standard headers and programs
dnl =======================================
AX_COMPILER_VENDOR
AX_COMPILER_VERSION
AC_PROG_SED
AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])



dnl ==================================
dnl Set CXXFLAGS, CPPFLAGS and LDFLAGS
dnl ==================================
AS_IF([test "x${enable_flags_setting}" = "xyes" && test "x${enable_debug}" = "xno"],
	[
	AX_APPEND_COMPILE_FLAGS([${cxxflags_test} -Wno-narrowing], [CXXFLAGS])
	CXXFLAGS=$( echo ${CXXFLAGS} | $SED -e 's/^ *//' -e 's/ *$//' )
	
	AX_APPEND_COMPILE_FLAGS([${cppflags_test}], [CPPFLAGS])
	CPPFLAGS=$( echo ${CPPFLAGS} | $SED -e 's/^ *//' -e 's/ *$//' )
	
	AX_APPEND_LINK_FLAGS([${ldflags_test}], [LDFLAGS])
	LDFLAGS=$( echo ${LDFLAGS} | $SED -e 's/^ *//' -e 's/ *$//' )
	]
)



dnl ======================
dnl Check for dependencies
dnl ======================
dnl zlib
PKG_CHECK_MODULES([ZLIB], [zlib])

dnl libarchive
PKG_CHECK_MODULES([LIBARCHIVE], [libarchive])

dnl SAMPLERATE
PKG_CHECK_MODULES([SAMPLERATE], [samplerate])

dnl SDL2
PKG_CHECK_MODULES([SDL2], [sdl2 >= 2.0.12])

dnl LibEpoxy
PKG_CHECK_MODULES([LIBEPOXY], [epoxy])

AC_CHECK_PROG(FLTKCONFIG,fltk-config,[fltk-config],[no])
test "$FLTKCONFIG" == "no" && AC_MSG_ERROR([Cannot find the fltk-config executable. Is FLTK installed?])

AC_SUBST(FLTK_CFLAGS,"$(fltk-config --use-gl --use-images --cxxflags)")
AS_CASE([${host_os}],
	[darwin*],
	[AC_SUBST(FLTK_LIBS,"$(fltk-config --use-gl --use-images --ldflags)")],
	[AC_SUBST(FLTK_LIBS,"-lGL $(fltk-config --use-gl --use-images --ldflags)")]
)

dnl full HTML suite
AC_ARG_ENABLE([doc],
	AS_HELP_STRING([--enable-doc], [Install full HTML documentation]))
AM_CONDITIONAL([ENABLE_FULL_HTML], [test "x$enable_doc" = "xyes"])



dnl ===================
dnl Initialise Automake
dnl ===================
AM_INIT_AUTOMAKE([1.15 foreign dist-bzip2 no-dist-gzip subdir-objects silent-rules])
AM_SILENT_RULES([yes])



dnl ========
dnl Finalise
dnl ========
AC_CONFIG_FILES([Makefile])
AC_OUTPUT



dnl ============================
dnl Report configuration to user
dnl ============================
AC_MSG_RESULT([
	${PACKAGE_NAME} ${VERSION}
	Using ${ax_cv_cxx_compiler_vendor}, ${ax_cv_cxx_compiler_version}

	CXX:        ${CXX}
	CXXFLAGS:   ${CXXFLAGS}
	CPPFLAGS:   ${CPPFLAGS}
	LDFLAGS:    ${LDFLAGS}
])
