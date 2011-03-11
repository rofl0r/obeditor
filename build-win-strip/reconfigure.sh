if test "x$1" = "x"; then
	../configure  --host=i586-mingw32msvc --prefix=/usr/i586-mingw32msvc/
else
	../configure  --host=i586-mingw32msvc --prefix=/usr/i586-mingw32msvc/ --enable-debug
fi