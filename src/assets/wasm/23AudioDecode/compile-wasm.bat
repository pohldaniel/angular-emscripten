emcc^
 src/animation/BoneDescription.cpp^
 src/animation/Bone.cpp^
 src/animation/Animation.cpp^
 src/animation/AnimationState.cpp^
 src/animation/AnimatedModel.cpp^
 src/animation/AnimationController.cpp^
 src/BinaryIO.cpp^
 src/Fade.cpp^
 src/Mouse.cpp^
 src/Keyboard.cpp^
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
 Nuklear/NkContext.cpp^
 Nuklear/NkStyle.cpp^
 Nuklear/NkCalculator.cpp^
 Nuklear/NkNodeEditor.cpp^
 Nuklear/NkJoystick.cpp^
 Sound/SoftwareMixer.cpp^
 Sound/EMPlayer.cpp^
 Sound/OpenALPlayer.cpp^
 Sound/SoundDevice.cpp^
 Sound/AudioDecoder.cpp^
 states/StateMachine.cpp^
 states/AudioDecode.cpp^
 libimgui.a^
 ../lib/libfreeimageT.a^
 ../lib/libassimpT.a^
 ../lib/libzlib.a^
 ../lib/libavcodec.a^
 ../lib/libavutil.a^
 ../lib/libavformat.a^
 ../lib/libswresample.a^
 -I "./" -I "./include" -I "./include/animation" -I "./libimgui" -I "../include/glm" -I "../include/FreeImage" -I "../include/rapidjson/include" -I "../include/ffmpeg" -I "../include"^
 -Wall -Wno-return-type-c-linkage -Wno-missing-braces -Wunused-result^
 -DFREEIMAGE_LIB -DWEBGPU_BACKEND=WGPU^
 -o webgpu.js -lopenal -std=c++17 -Os --use-port=contrib.glfw3 --use-port=emdawnwebgpu --preload-file res/models --preload-file res/shader --preload-file res/textures --preload-file res/fonts --preload-file res/sounds -s ASYNCIFY -s ASSERTIONS -s TOTAL_MEMORY=167772160 -s WASM_WORKERS=1 -s AUDIO_WORKLET=1 -s SHARED_MEMORY=1 -pthread -s ALLOW_MEMORY_GROWTH=1 -s "EXPORTED_FUNCTIONS=['_main']" -s "EXPORTED_RUNTIME_METHODS=['ccall']" -s MODULARIZE=1^
 -s EXPORT_NAME='AudioDecodeModule'^
 && exit
