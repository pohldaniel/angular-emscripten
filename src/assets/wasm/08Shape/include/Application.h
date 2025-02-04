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
    static GLFWwindow* Window;

private:

	void initWindow();
	void initImGUI();
	void initOpenGL();
	void initStates();
	void messageLopp();

	float& fdt;
	float& dt;

	double last;
	 
	static StateMachine* Machine;
};