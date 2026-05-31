emcc^
 src/animation/BoneDescription.cpp^
 src/animation/Bone.cpp^
 src/animation/Animation.cpp^
 src/animation/AnimationState.cpp^
 src/animation/AnimatedModel.cpp^
 src/BinaryIO.cpp^
 src/Fade.cpp^
 src/Mouse.cpp^
 src/Material.cpp^
 src/Transform.cpp^
 src/Mesh.cpp^
 src/Model.cpp^
 src/ObjModel.cpp^
 src/AssimpModel.cpp^
 src/Camera.cpp^
 src/CharacterSet.cpp^
 src/Application.cpp^
 src/main.cpp^
 Shape/Capsule.cpp^
 Shape/Cube.cpp^
 Shape/Cylinder.cpp^
 Shape/Quad.cpp^
 Shape/Segment.cpp^
 Shape/Sphere.cpp^
 Shape/Spiral.cpp^
 Shape/Torus.cpp^
 Shape/TorusKnot.cpp^
 Shape/Shape.cpp^
 WebGPU/WgpContext.cpp^
 WebGPU/WgpTexture.cpp^
 WebGPU/WgpBuffer.cpp^
 WebGPU/WgpMesh.cpp^
 WebGPU/WgpModel.cpp^
 WebGPU/WgpBatchRenderer.cpp^
 WebGPU/WgpFontRenderer.cpp^
 WebGPU/WgpRenderer.cpp^
 states/StateMachine.cpp^
 states/DefferedRendering.cpp^
 libimgui.a^
 ../lib/libfreeimage.a^
 ../lib/libassimp.a^
 ../lib/libzlib.a^
 -I "./" -I "./include" -I "./include/animation" -I "./libimgui" -I "../include/glm" -I "../include/FreeImage" -I "../include/rapidjson/include" -I "../include/assimp"^
 -Wall -Wno-return-type-c-linkage^
 -DFREEIMAGE_LIB -DWEBGPU_BACKEND=WGPU^
 -o webgpu.js -std=c++17 -Wno-missing-braces -Wunused-result -Os --use-port=contrib.glfw3 -s USE_WEBGPU=1 --preload-file res/shader --preload-file res/models -s ASYNCIFY -s ASSERTIONS -s TOTAL_MEMORY=67108864 -s ALLOW_MEMORY_GROWTH=1 -s "EXPORTED_FUNCTIONS=['_main']" -s "EXPORTED_RUNTIME_METHODS=['ccall']" -s MODULARIZE=1^
 -s EXPORT_NAME='DefferedRenderingModule'^
 && exit
