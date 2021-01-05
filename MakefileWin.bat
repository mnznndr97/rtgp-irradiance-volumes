@echo off
IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\VC\Auxiliary\Build\vcvarsall.bat" x64
) ELSE (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\VC\Auxiliary\Build\vcvarsall.bat" x64
)

set compilerflags=/Od /Z7 /EHsc /MT /std:c++17 /DDEBUG 

REM set MyVS="C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\VC\Tools\MSVC\14.28.29213"
REM set _LINK_=/debug -incremental:no /wholearchive:%MyVS%\lib\x64\clang_rt.asan-x86_64.lib /wholearchive:%MyVS%\lib\x64\clang_rt.asan_cxx-x86_64.lib
REM set path=%path%;%MyVS%\bin\Hostx64\x64
REM set compilerflags=%compilerflags% -fsanitize=address

set includedirs=/Iinclude
set linkerflags=/LIBPATH:libs/win glfw3.lib assimp-vc142-mt.lib zlib.lib IrrXML.lib gdi32.lib user32.lib Shell32.lib  freetype.lib
cl.exe %compilerflags% %includedirs% include/threading/Interlocked.cpp include/glad/glad.c main.cpp /Fe:IrradianceVolumes.exe /link %linkerflags% 
