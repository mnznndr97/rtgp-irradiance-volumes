@echo off
IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\VC\Auxiliary\Build\vcvarsall.bat" x64
) ELSE (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\VC\Auxiliary\Build\vcvarsall.bat" x64
)
set compilerflags=/O2i /Zi /Zo /EHsc /MT /std:c++17 /DNDEBUG 
set includedirs=/Iinclude
set linkerflags=/LIBPATH:libs/win glfw3.lib assimp-vc142-mt.lib zlib.lib IrrXML.lib gdi32.lib user32.lib Shell32.lib  freetype.lib
cl.exe %compilerflags% %includedirs% include/glad/glad.c include/threading/Interlocked.cpp main.cpp /Fe:IrradianceVolumes.exe /link %linkerflags% 
