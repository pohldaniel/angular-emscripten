emcc^
 libimgui/imgui.cpp^
 libimgui/imgui_draw.cpp^
 libimgui/imgui_impl_glfw.cpp^
 libimgui/imgui_impl_wgpu.cpp^
 libimgui/imgui_tables.cpp^
 libimgui/imgui_widgets.cpp^
 -I "./include"^
 -DISABLE_EXCEPTION_CATCHING=0^
 --use-port=emdawnwebgpu -c -s -std=c++14 -Wall -Wextra -Wpedantic -Oz^
 && exit