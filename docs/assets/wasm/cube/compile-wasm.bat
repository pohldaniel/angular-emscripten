emcc -Os cube.c libsoil.a -o cube.js -s LEGACY_GL_EMULATION=1 -I SOIL/src -s EXPORTED_RUNTIME_METHODS="['ccall']" -s FORCE_FILESYSTEM=1 -s MODULARIZE=1 -s EXPORT_NAME='CubeModule' && exit
