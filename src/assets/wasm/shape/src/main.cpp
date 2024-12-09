#include <emscripten.h>
#include "Application.h"

int main(int argc, const char* argv[]) {
  Application application;

  //while(application.isRunning()){
  //  application.update();
  //  application.render();
  //}

  emscripten_set_main_loop_arg(Application::MessageLopp, &application, 0, true);
  return 0;
}