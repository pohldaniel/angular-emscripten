emcc main.cpp imgui.a -o ImguiWeb.js -Wall -std=c++14 -Wno-missing-braces -Wunused-result -Os -I libimgui -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 -s "EXPORTED_FUNCTIONS=['_free','_malloc','_main']" -s "EXPORTED_RUNTIME_METHODS=['ccall']" -s MODULARIZE=1 -s EXPORT_NAME='ImguiWebModule' && exit