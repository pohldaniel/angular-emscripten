emcc src/Shader.cpp^
 src/Application.cpp^
 src/main.cpp^
 states/Shape.cpp^
 states/StateMachine.cpp^
 src/ResourceManager.cpp^
 src/WebGpuUtils.cpp^
 glfw3webgpu.a^
 libimgui.a^
 -I "./" -I "./include" -I "./glm" -I "./libimgui" -I "./glfw3webgpu"^
 -Wall^
 -o webgpu.js -std=c++17 -Wno-missing-braces -Wunused-result -Os --use-port=contrib.glfw3 -s USE_WEBGPU=1 --preload-file res/shader --preload-file res/models --preload-file res/textures -s ASYNCIFY -s ASSERTIONS -s TOTAL_MEMORY=67108864 -s ALLOW_MEMORY_GROWTH=1 -s "EXPORTED_FUNCTIONS=['_main']" -s "EXPORTED_RUNTIME_METHODS=['ccall']" -s MODULARIZE=1^
 -s EXPORT_NAME='AdditionalModule'^
