@echo off

pushd %~dp0\..\..\

call vendor\premake\premake5.exe clean

popd
PAUSE