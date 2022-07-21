@echo off

set rootDir=%cd%
if not exist build (
    mkdir build
)

set debug=true

if %debug%==true (
    echo Compiling in debug mode.
    echo.

    set debugFlags = -DGAME_DEBUG -D_DEBUG
    set entryPoint = /link /subsystem:CONSOLE
) else (
    echo Compiling in release mode.
    echo.

    set debugFlags=-DNDEBUG -O2 -Oi -fp:fast
    set entryPoint = /link /subsystem:WINDOWS
)

set output=Backrooms
set flags=-nologo -FC -Zi -WX -W4 /MP
set disabledWarnings=-wd4100 -wd4201 -wd4018 -wd4099 -wd4189 -wd4505 -wd4530 -wd4840 -wd4324 -wd4459 -wd4702 -wd4244 -wd4310 -wd4611 -wd4996
set source=%rootDir%/game/*.cpp
set links=user32.lib ole32.lib d3d11.lib d3dcompiler.lib dxgi.lib dr_libs.lib
set includeDirs= -I%rootDir%/vendor

pushd build
if not exist dr_libs.lib (
    cl -nologo -FC -Zi -w /MP -Fodr_libs %rootDir%/vendor/dr_libs/dr_libs.c /incremental /c
    lib %rootDir%/build/dr_libs.obj
)

cl %disabledWarnings% %includeDirs% %debugFlags% %flags% -Fe%output% %source% /std:c++latest /incremental %links% %entryPoint% 
popd

echo.
echo Build finished.