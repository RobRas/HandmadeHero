@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x64

mkdir ..\..\build
pushd ..\..\build
cl -Zi ..\HandmadeHero\code\win32_handmade.cpp user32.lib
popd