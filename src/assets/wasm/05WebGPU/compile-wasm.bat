emcc src/Mouse.cpp^
 src/Material.cpp^
 src/Transform.cpp^
 src/ObjModel.cpp^
 src/Camera.cpp^
 src/Application.cpp^
 src/main.cpp^
 WebGPU/WgpContext.cpp^
 WebGPU/WgpTexture.cpp^
 WebGPU/WgpBuffer.cpp^
 WebGPU/WgpMesh.cpp^
 WebGPU/WgpModel.cpp^
 states/Shape.cpp^
 states/Default.cpp^
 states/StateMachine.cpp^
 src/WebGpuUtils.cpp^
 glfw3webgpu.a^
 libimgui.a^
 libfreeimage.a^
 -I "./" -I "./include" -I "./glm" -I "./glfw3webgpu" -I "./FreeImage"  -I "./libimgui"^
 -o web-gpu.js -Wall -std=c++17 -Wno-missing-braces -Wunused-result -Os -DFREEIMAGE_LIB -DWEBGPU_BACKEND=WGPU --use-port=contrib.glfw3 -s USE_WEBGPU=1 --preload-file res/shader --preload-file res/textures --preload-file res/models -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s ALLOW_MEMORY_GROWTH=1 -s "EXPORTED_FUNCTIONS=['_main']" -s "EXPORTED_RUNTIME_METHODS=['ccall']" -s MODULARIZE=1 -s EXPORT_NAME='WebGPUModule'
