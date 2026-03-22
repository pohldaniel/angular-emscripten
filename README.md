# angular-emscripten

This repository shows how to integrate emscripten wasm-modules inside an angular 21 application. The modules are compiled with emscripten version 3.1.74 under Windows 11. There .bat files which I have used for compiling the examples. For building libfreeimge.a have a look at https://github.com/pohldaniel/FreeImage_emscripten and run compile.bat and link.bat. Some useful repositories I have visited as well https://github.com/cwoffenden/hello-webgpu, https://github.com/boyanio/angular-wasm and https://github.com/eliemichel/LearnWebGPU-Code/tree/main. 

Try it out: https://pohldaniel.github.io/angular-emscripten/

For the GitHub deployment use the command:

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;npm run build-prod

For the local Setup use the commands:

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;npm isntall  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ng serve

For building Freetype download the make binaries from https://gnuwin32.sourceforge.net/packages/make.htm and set the Environmentvariable to the bin folder

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;mkdir build
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cd build
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;emcmake cmake .. -G "Unix Makefiles"
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;emmake make
