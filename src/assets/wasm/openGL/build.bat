emcc src/Application.cpp src/glError.cpp src/main.cpp src/MyApplication.cpp src/Shader.cpp -o open-gl.js -Wall -std=c++14 -Wno-missing-braces -Wunused-result -Os -I include -I "C:/OpenGL/lib/glm" --use-port=contrib.glfw3 -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 --preload-file shader -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s "EXPORTED_FUNCTIONS=['_free','_malloc','_main']" -s "EXPORTED_RUNTIME_METHODS=['ccall']" -s MODULARIZE=1 -s EXPORT_NAME='OpenModule'

cmd /k