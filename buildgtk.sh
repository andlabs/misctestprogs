[ X"$1" == X-s ] && {
	CC="clang -Wno-incompatible-pointer-types"
	shift
}
${CC:-gcc} -o `basename "$1" .c` "$@" `pkg-config --cflags --libs gtk+-3.0`
