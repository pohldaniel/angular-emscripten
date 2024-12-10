#include <iostream>
#include <States/StateMachine.h>

struct GLFWwindow;
class Application {

public:
	Application(float& dt, float& fdt);
	~Application();

	//void update();
	//void render();
	bool isRunning();

	static void MessageLopp(void *arg);

	static int Width;
	static int Height;

private:

	void initWindow();
	void initOpenGL();
	void initStates();
	void messageLopp();

	float& fdt;
	float& dt;

	double begin;
	double end;
	
    static GLFWwindow* Window;
	static StateMachine* Machine;
};