@echo off

pushd %~dp0\..\..\

call vendor\premake\premake5.exe gmake2 os=windows

popd
PAUSE