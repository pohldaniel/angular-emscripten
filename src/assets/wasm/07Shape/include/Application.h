#include <iostream>
#include <States/StateMachine.h>

struct GLFWwindow;
class Application {

public:
	Application(float& dt, float& fdt);
	~Application();

	bool isRunning();

	static void MessageLopp(void *arg);

	static int Width;
	static int Height;
	static double Time;

private:

	void initWindow();
	void initOpenGL();
	void initStates();
	void messageLopp();

	float& fdt;
	float& dt;

	double last;
	
    static GLFWwindow* Window;
	static StateMachine* Machine;
};