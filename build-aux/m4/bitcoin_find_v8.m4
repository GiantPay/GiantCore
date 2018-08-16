AC_DEFUN([BITCOIN_FIND_V8],[
  AC_MSG_CHECKING([for V8])
  V8_CPPFLAGS=
  V8_LIBS = '-L/opt/libv8-6.6/lib/libicui18n.so '
  V8_LIBS += '-L/opt/libv8-6.6/lib/libicuuc.so'
  V8_LIBS += '-L/opt/libv8-6.6/lib/libv8_libbase.so '
  V8_LIBS += '-L/opt/libv8-6.6/lib/libv8_libplatform.so '
  V8_LIBS += '-L/opt/libv8-6.6/lib/libv8.so '
])
