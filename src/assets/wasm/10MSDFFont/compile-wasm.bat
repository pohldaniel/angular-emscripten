emcc src/Mouse.cpp^
 src/Material.cpp^
 src/Transform.cpp^
 src/ObjModel.cpp^
 src/Camera.cpp^
 src/CharacterSet.cpp^
 src/Application.cpp^
 src/main.cpp^
 shape/Cube.cpp^
 shape/Sphere.cpp^
 shape/Torus.cpp^
 shape/TorusKnot.cpp^
 shape/Spiral.cpp^
 shape/Shape.cpp^
 WebGPU/WgpContext.cpp^
 WebGPU/WgpTexture.cpp^
 WebGPU/WgpBuffer.cpp^
 WebGPU/WgpMesh.cpp^
 WebGPU/WgpModel.cpp^
 WebGPU/WgpBatchRenderer.cpp^
 WebGPU/WgpFontRenderer.cpp^
 states/NormalMap.cpp^
 states/MSDFFont.cpp^
 states/StateMachine.cpp^
 libimgui.a^
 libfreeimage.a^
 -I "./" -I "./include" -I "./shape" -I "./glm" -I "./libimgui" -I "./FreeImage" -I "./rapidjson/include"^
 -Wall^
 -DFREEIMAGE_LIB -DWEBGPU_BACKEND=WGPU^
 -o webgpu.js -std=c++17 -Wno-missing-braces -Wunused-result -Os --use-port=contrib.glfw3 -s USE_WEBGPU=1 --preload-file res/shader --preload-file res/models --preload-file res/textures --preload-file res/fonts -s ASYNCIFY -s ASSERTIONS -s TOTAL_MEMORY=67108864 -s ALLOW_MEMORY_GROWTH=1 -s "EXPORTED_FUNCTIONS=['_main']" -s "EXPORTED_RUNTIME_METHODS=['ccall']" -s MODULARIZE=1^
 -s EXPORT_NAME='MSDFFontModule'^
 && exit
