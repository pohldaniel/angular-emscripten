#include <iostream>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

struct GLFWwindow;
class Application {

public:
	Application();
	~Application();

	void update();
	void render();
	bool isRunning();

	static void MessageLopp(void *arg);

	static int Width;
	static int Height;
	
private:

	void initWindow();
	void initWebGPU();
	void messageLopp();
	WGPUTextureView GetNextSurfaceTextureView();

	WGPUDevice device;
	WGPUQueue queue;
	WGPUSurface surface;
	WGPUAdapter adapter;
	WGPUInstance instance;

    static GLFWwindow* Window;
};