call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvars64.bat"
chdir /d %~dp0

set include_dirs=/I"C:/dev/libs/" ^
    /I"C:/dev/libs/sdl2/include/" ^
    /I"C:/dev/libs/glad/include/"

set link_path=/LIBPATH:"C:/dev/libs/sdl2/lib/x64" /LIBPATH:"C:/dev/libs/glad/static/"
set link_libs=SDL2.lib SDL2main.lib glad.lib

cl src/main.cpp /Fegame.exe %include_dirs% /DAV_GL_GLAD ^
    /EHsc /Zi ^
    /link %link_path%  %link_libs% /SUBSYSTEM:WINDOWS
