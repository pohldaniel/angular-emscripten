emcc src/Application.cpp src/main.cpp src/webgpu-utils.cpp glfw3webgpu/glfw3webgpu.a -o shape.js -Wall -std=c++14 -Wno-missing-braces -Wunused-result -Os -I include -I "./glm" -I "./glfw3webgpu" -DWEBGPU_BACKEND=WGPU --use-port=contrib.glfw3 -s USE_WEBGL2=1 -s USE_WEBGPU=1 -s MIN_WEBGL_VERSION=2 --preload-file res/shader -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s "EXPORTED_FUNCTIONS=['_free','_malloc','_main']" -s "EXPORTED_RUNTIME_METHODS=['ccall']" -s MODULARIZE=1 -s EXPORT_NAME='ShapeModule'

cmd /k

emcc src/Application.cpp src/main.cpp src/webgpu-utils.cpp glfw3webgpu/glfw3webgpu.a -o shape.js -Wall -std=c++14 -Wno-missing-braces -Wunused-result -Os --use-port=contrib.glfw3 -I "./include" -I "./glfw3webgpu" -s ALLOW_MEMORY_GROWTH=0 -s ENVIRONMENT=web -s NO_EXIT_RUNTIME=1 -s NO_FILESYSTEM=1 -s STRICT=1 -s TEXTDECODER=2 -s USE_WEBGPU=1 -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME='ShapeModule'


emcc src/Application.cpp src/main.cpp src/webgpu-utils.cpp glfw3webgpu/glfw3webgpu.a -o shape.js -Wall -std=c++14 -Wno-missing-braces -Wunused-result -Os -I include -I "./glm" -I "./glfw3webgpu" -s ALLOW_MEMORY_GROWTH=0 -s ENVIRONMENT=web --use-port=contrib.glfw3 -s USE_WEBGPU=1 -s WASM=1 -s MODULARIZE=1 -s ASYNCIFY -s "EXPORTED_FUNCTIONS=['_free','_malloc','_main']" -s "EXPORTED_RUNTIME_METHODS=['ccall']" -s MODULARIZE=1 -s EXPORT_NAME='ShapeModule'



emcc -c -s DISABLE_EXCEPTION_CATCHING=0 -std=c11 -Wall -Wextra -Wpedantic -Oz glfw3webgpu.c

emar rcs glfw3webgpu.a glfw3webgpu.o