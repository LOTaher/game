@echo off
setlocal

set BASE=base
set SHARED=shared
set RAYLIB=vendor\raylib

set BASE_SRC=%BASE%\lt_arena.c %BASE%\lt_net.c %BASE%\lt_string.c %BASE%\lt_os_win32.c %BASE%\lt_net_win32.c

set TARGET=%1
if "%TARGET%"=="" set TARGET=all

if "%TARGET%"=="directory" goto :directory
if "%TARGET%"=="client"    goto :client
if "%TARGET%"=="server"    goto :server
if "%TARGET%"=="all"       goto :all

echo Unknown target: %TARGET%
echo Usage: build.bat [directory^|client^|server^|all]
exit /b 1

:all
call :directory
call :client
call :server
goto :eof

:directory
cl /nologo /Zi /I%BASE% /I%SHARED% ^
   directory\main.c %BASE_SRC% ^
   /Fe:directory.exe ^
   /link ws2_32.lib user32.lib
goto :eof

:client
cl /nologo /Zi /I%BASE% /I%SHARED% /I%RAYLIB%\include ^
   client\main.c %BASE_SRC% ^
   /Fe:client.exe ^
   /link /LIBPATH:%RAYLIB%\lib raylib.lib opengl32.lib gdi32.lib winmm.lib shell32.lib user32.lib ws2_32.lib
goto :eof

:server
cl /nologo /Zi /I%BASE% /I%SHARED% ^
   server\main.c %BASE_SRC% ^
   /Fe:server.exe ^
   /link ws2_32.lib user32.lib
goto :eof
