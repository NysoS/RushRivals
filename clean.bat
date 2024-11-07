@echo off

if EXIST "./Saved/*.*" (
    rmdir /s /q Saved
)

if EXIST "./Build/*.*" (
    rmdir /s /q Build
)

if EXIST "./Intermediate/*.*" (
    rmdir /s /q Intermediate
)

if EXIST "./DerivedDataCache/*.*" (
    rmdir /s /q DerivedDataCache
)

if EXIST "./.idea/*.*" (
    rmdir /s /q .idea
)

if EXIST "./.vs/*.*" (
    rmdir /s /q .vs
)

if EXIST "./*.vsconfig" (
    del /q .vsconfig
)

if EXIST "./*.sln" (
    del /q *.sln
)