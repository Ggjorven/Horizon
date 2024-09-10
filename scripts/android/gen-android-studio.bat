@echo off

pushd %~dp0\..\..\

call vendor\premake\premake5.exe android-studio --os=android

popd
PAUSE