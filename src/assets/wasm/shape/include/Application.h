#include <iostream>

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
	void initOpenGL();
	void messageLopp();
	
    static GLFWwindow* Window;
};