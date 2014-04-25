${CC:-clang} -g -o `basename "$1" .m` "$@" -framework Foundation -framework AppKit --std=c99
