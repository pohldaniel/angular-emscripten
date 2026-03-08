#include <emscripten.h>
#include <emscripten/html5.h>
#include "Application.h"

extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void Resize(int width, int height){
    Application::Resize(width, height);
  }

  EMSCRIPTEN_KEEPALIVE
  bool IsInitialized(){
    return Application::IsInitialized();
  }

  EMSCRIPTEN_KEEPALIVE
  void Cleanup(){
    emscripten_cancel_main_loop();
    Application::Cleanup();
  }

  EMSCRIPTEN_KEEPALIVE
  int GetWidth(){
    return Application::Width;
  }

  EMSCRIPTEN_KEEPALIVE
  int GetHeight(){
    return Application::Height;
  }
}

EM_BOOL OnWindowResize(int eventType, const EmscriptenUiEvent* e, void* userdata){
  //Application* application  = reinterpret_cast<Application*>(userdata);
  int width, height;
  emscripten_get_canvas_element_size("#canvas", &width, &height);
  Application::Resize(width, height);

  return EM_FALSE;
}

int main(int argc, const char* argv[]) {
  float deltaTime = 0.0f;
	float fixedDeltaTime = 0.0f;
  Application application(deltaTime, fixedDeltaTime);

  //while(application.isRunning()){
  //  application.update();
  //  application.render();
  //}

  //emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &application, EM_FALSE, OnWindowResize);
  emscripten_set_main_loop_arg(Application::MessageLopp, &application, 0, true);
  return 0;
}