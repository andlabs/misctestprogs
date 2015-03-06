i686-w64-mingw32-gcc -g -o test.dll -shared -Wl,--out-implib,test.lib dll.c -static-libgcc -luser32 -lkernel32 -lgdi32 -m32
i686-w64-mingw32-gcc -g -o test.exe main.c test.lib -static-libgcc -luser32 -lkernel32 -lgdi32 -m32
