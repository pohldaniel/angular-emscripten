#pragma once
#include <iostream>
#include <webgpu/webgpu.h>
#include <States/StateMachine.h>

struct GLFWwindow;

class Application {

	friend void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	friend void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	friend void glfwMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);

public:
	Application(float& dt, float& fdt);
	~Application();

	bool isRunning();

	static int Width;
	static int Height;
	
	static void MessageLopp(void *arg);
	static GLFWwindow* Window;

private:

	void initWindow();
	void initWebGPU();
	void initImGUI();
	void initStates();
	void messageLopp();
	
	float& fdt;
	float& dt;
	double last;
	
	static StateMachine* Machine;
	static double Time;
};