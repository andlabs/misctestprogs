[ X"$1" == X-64 ] && {
	CC="x86_64-w64-mingw32-gcc"
	shift
}
${CC:-i686-w64-mingw32-gcc} -g -o `basename "$1" .c`.exe "$@" comctl6.c --std=c99 -luser32 -lkernel32 -lcomdlg32 -lgdi32 -lmsimg32 -lshell32 -ladvapi32
