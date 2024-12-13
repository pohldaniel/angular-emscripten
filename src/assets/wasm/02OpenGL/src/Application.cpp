#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <States/Shape.h>

#include "Application.h"

GLFWwindow* Application::Window = nullptr;
StateMachine* Application::Machine = nullptr;

int Application::Width;
int Application::Height;
double Application::Time;

void Application::MessageLopp(void *arg) {
	Application* application  = reinterpret_cast<Application*>(arg);

    Time = glfwGetTime();
    application->dt = float(Time - application->last);
    application->last = Time;

	application->messageLopp();
}

Application::Application(float& dt, float& fdt) : fdt(fdt), dt(dt), last(0.0) {
    Application::Width = 640;
    Application::Height = 480;
	initWindow();
	initOpenGL();
    initStates();
}

Application::~Application() {
	glfwDestroyWindow(Window);
	glfwTerminate();
}

void Application::initWindow() {
    if (!glfwInit()) {
        throw std::runtime_error("Couldn't init GLFW");
    }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    Window = glfwCreateWindow(640, 480, "Shape", NULL, NULL);
    if (!Window) {
        glfwTerminate();
        throw std::runtime_error("Couldn't create a window");
    }
}

void Application::initOpenGL(){
    glfwMakeContextCurrent(Window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if (err != GLEW_OK) {
        glfwTerminate();
        throw std::runtime_error(std::string("Could initialize GLEW, error = ") +
                             (const char*)glewGetErrorString(err));
    }

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported " << version << std::endl;

    // opengl configuration
    glEnable(GL_DEPTH_TEST);  // enable depth-testing
    glDepthFunc(GL_LESS);  // depth-testing interprets a smaller value as "closer"
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
}

bool Application::isRunning(){
    messageLopp();
    return glfwWindowShouldClose(Window);
}

void Application::messageLopp(){
    glfwPollEvents();
    Machine->update();
    Machine->render();
}

void Application::initStates(){
    Machine = new StateMachine(dt, fdt);
    Machine->addStateAtTop(new Shape(*Machine));
}