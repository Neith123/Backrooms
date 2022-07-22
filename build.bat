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
set links=user32.lib ole32.lib d3d11.lib d3dcompiler.lib dxgi.lib dr_libs.lib cgltf.lib stb_image.lib imgui.lib
set includeDirs= -I%rootDir%/vendor

pushd build
if not exist dr_libs.lib (
    cl -nologo -FC -Zi -w /MP -Fodr_libs %rootDir%/vendor/dr_libs/dr_libs.c /incremental /c
    lib %rootDir%/build/dr_libs.obj
)
if not exist cgltf.lib (
    cl -nologo -FC -Zi -w /MP -Focgltf %rootDir%/vendor/cgltf/cgltf.c /incremental /c
    lib %rootDir%/build/cgltf.obj
)
if not exist stb_image.lib (
    cl -nologo -FC -Zi -w /MP -Fostb_image %rootDir%/vendor/stb/stb_image.c /incremental /c
    lib %rootDir%/build/stb_image.obj
)
if not exist imgui.lib (
    cl -nologo -FC -Zi -w /MP -Fe %rootDir%/vendor/imgui/*.cpp /incremental /c
    lib %rootDir%/build/imgui.obj %rootDir%/build/imgui_demo.obj %rootDir%/build/imgui_draw.obj %rootDir%/build/imgui_impl_dx11.obj %rootDir%/build/imgui_impl_win32.obj %rootDir%/build/imgui_tables.obj %rootDir%/build/imgui_widgets.obj
)

cl %disabledWarnings% %includeDirs% %debugFlags% %flags% -Fe%output% %source% /std:c++latest /incremental %links% %entryPoint% 
popd

echo.
echo Build finished.