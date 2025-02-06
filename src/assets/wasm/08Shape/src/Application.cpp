#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <States/ShapeState.h>
#include "Mouse.h"
#include "Application.h"
#include "Event.h"

GLFWwindow* Application::Window = nullptr;
StateMachine* Application::Machine = nullptr;

int Application::Width;
int Application::Height;
double Application::Time;

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void glfwMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);

void Application::MessageLopp(void *arg) {
	Application* application  = reinterpret_cast<Application*>(arg);

    Time = glfwGetTime();
    application->dt = float(Time - application->last);
    application->last = Time;

	application->messageLopp();
}

Application::Application(float& dt, float& fdt) : fdt(fdt), dt(dt), last(0.0) {
    Application::Width = 1260;
    Application::Height = 720;
	initWindow();
	initOpenGL();
    initImGUI();
    initStates();

    glfwSetWindowUserPointer(Window, this);
    glfwSetKeyCallback(Window, glfwKeyCallback);
    glfwSetMouseButtonCallback(Window, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(Window, glfwMouseMoveCallback);
}

Application::~Application() {
    //ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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

    Window = glfwCreateWindow(Application::Width, Application::Height, "Shape", NULL, NULL);
    if (!Window) {
        glfwTerminate();
        throw std::runtime_error("Couldn't create a window");
    }

    //glViewport(0, 0, Application::Width, Application::Height);
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

void Application::initImGUI() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOther(Window, true);
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(Window, "#canvas");
    ImGui_ImplOpenGL3_Init("#version 300 es");
}

bool Application::isRunning(){
    messageLopp();
    return glfwWindowShouldClose(Window);
}

void Application::messageLopp(){
    glfwPollEvents();
    Mouse::instance().update();
    Machine->update();
    Machine->render();
}

void Application::initStates(){
    Machine = new StateMachine(dt, fdt);
    Machine->addStateAtTop(new ShapeState(*Machine));
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  //Application* instance = (Application *) glfwGetWindowUserPointer(window);
  //instance->OnKey(key, scancode, action, mods);

  //Machine->getStates().top()->
}

void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    Event event;
    event.type = Event::MOUSEBUTTONDOWN;
    event.data.mouseButton.x = static_cast<int>(xpos);
    event.data.mouseButton.y = static_cast<int>(ypos);
    event.data.mouseButton.button = (button == GLFW_MOUSE_BUTTON_RIGHT) ? Event::MouseButtonEvent::MouseButton::BUTTON_RIGHT : Event::MouseButtonEvent::MouseButton::BUTTON_LEFT;

    if (action == GLFW_PRESS)
        Application::Machine->getStates().top()->OnMouseButtonDown(event.data.mouseButton);

    if(action == GLFW_RELEASE)
        Application::Machine->getStates().top()->OnMouseButtonUp(event.data.mouseButton);
}

void glfwMouseMoveCallback(GLFWwindow* window, double xpos, double ypos){
    Event event;
    event.type = Event::MOUSEMOTION;
    event.data.mouseMove.x = static_cast<int>(xpos);
    event.data.mouseMove.y = static_cast<int>(ypos);
    //event.data.mouseMove.button = wParam & MK_RBUTTON ? Event::MouseMoveEvent::MouseButton::BUTTON_RIGHT : wParam & MK_LBUTTON ? Event::MouseMoveEvent::MouseButton::BUTTON_LEFT : Event::MouseMoveEvent::MouseButton::NONE;
    
    Application::Machine->getStates().top()->OnMouseMotion(event.data.mouseMove);
}
