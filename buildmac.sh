# TODO find out how to integrate the .dSYM/ folder into the binary itself
${CC:-clang} -g -o `basename "$1" .m` "$@" -framework Foundation -framework AppKit --std=c99
