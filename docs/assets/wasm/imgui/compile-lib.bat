emcc -c -s DISABLE_EXCEPTION_CATCHING=0 -std=c++14 -Wall -Wextra -Wpedantic -Oz libimgui/imgui.cpp libimgui/imgui_demo.cpp libimgui/imgui_draw.cpp libimgui/imgui_impl_glfw.cpp libimgui/imgui_impl_opengl3.cpp libimgui/imgui_tables.cpp libimgui/imgui_widgets.cpp && exit

rem echo BUILD
rem exit
rem link.bat

rem emar rcs imgui.a imgui.o imgui_demo.o imgui_draw.o imgui_impl_glfw.o imgui_impl_opengl3.o imgui_tables.o imgui_widgets.o
rem rm *.o
rem emcc main.cpp imgui.a -o imgui.js -Wall -std=c++14 -Wno-missing-braces -Wunused-result -Os -I. -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 -s "EXPORTED_FUNCTIONS=['_free','_malloc','_main']" -s "EXPORTED_RUNTIME_METHODS=['ccall']" -s MODULARIZE=1 -s EXPORT_NAME='ImguiModule'

rem cmd /k