@echo off
setlocal
pushd "%~dp0"

echo //-----------------------------------------------------------------------------
echo // File : version.h
echo // Desc : Application Version.
echo // Copyright(c) Project Asura. All right reserved.
echo //-----------------------------------------------------------------------------
echo #pragma once
echo #define APP_VERSION                       1
set /p DUMMY=#define APP_BUILD_VERSION                 <NUL
git rev-parse HEAD
echo #define APP_BUILD_VERSION_TO_TAG(x)       APP_BUILD_VERSION_TO_TAG_(x)
echo #define APP_BUILD_VERSION_TO_TAG_(x)      #x
echo #define APP_BUILD_VERSION_TAG             APP_BUILD_VERSION_TO_TAG(APP_BUILD_VERSION)

popd
