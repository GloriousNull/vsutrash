@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

set allowed_extensions=-Wno-gnu-pointer-arith -Wno-gnu-empty-initializer -Wno-microsoft-fixed-enum

set cpp_compiler_flags=-DDEBUG -std=c++2b -nostdlib++ -O0 -g -fno-exceptions -fno-rtti -ffast-math -fno-trapping-math -mavx2

set warnings=-Wall -Wextra -Wshadow -Wno-missing-braces -Wno-unused-function -Wpedantic -Wsign-compare
set c_compiler_flags=-DDEBUG -DAVX2 -std=c17 -O0 -g -ffast-math -fno-trapping-math -mavx2	
set linker_flags=-IE:\Dev\projects\gorgon\code\win -IE:\Dev\projects\gorgon\code\basic -IE:\Dev\projects\gorgon\include -luser32.lib -fuse-ld=lld -Wl,-subsystem:console

call time.bat clang %c_compiler_flags% %warnings% %allowed_extensions% %linker_flags% ../code/sec1.c -o sec1.exe

popd

