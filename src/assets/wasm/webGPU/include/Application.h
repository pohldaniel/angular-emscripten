#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>
#include <States/StateMachine.h>

struct GLFWwindow;
class Application {

public:
	Application(float& dt, float& fdt);
	~Application();

	bool isRunning();

	static void MessageLopp(void *arg);

	static WGPUTextureView GetNextSurfaceTextureView();

	static int Width;
	static int Height;
	static double Time;
	
	static WGPUDevice Device;
	static WGPUQueue Queue;

private:

	void initWindow();
	void initWebGPU();
	void initStates();
	void messageLopp();

	
	WGPUAdapter adapter;
	WGPUInstance instance;

	float& fdt;
	float& dt;

	double last;

    static GLFWwindow* Window;
	static StateMachine* Machine;
	static WGPUSurface Surface;
};