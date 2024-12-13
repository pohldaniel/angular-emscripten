#include <emscripten.h>
#include "Application.h"


int main(int argc, const char* argv[]) {
  
  float deltaTime = 0.0f;
	float fixedDeltaTime = 0.0f;
  Application application(deltaTime, fixedDeltaTime);

  //while(application.isRunning()){
  //  application.update();
  //  application.render();
  //}

  emscripten_set_main_loop_arg(Application::MessageLopp, &application, 0, true);
  return 0;
}