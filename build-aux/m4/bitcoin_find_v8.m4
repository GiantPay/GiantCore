AC_DEFUN([BITCOIN_FIND_V8],[
  AC_MSG_CHECKING([for V8])
  V8_LIBS="-L$depends_prefix/lib/v8 -lv8_monolith"

  LDFLAGS="$LDFLAGS $V8_LIBS"
  export LDFLAGS
])
