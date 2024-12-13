emcc -c -s DISABLE_EXCEPTION_CATCHING=0 -std=c11 -Wall -Wextra -Wpedantic -Oz SOIL/src/image_DXT.c SOIL/src/image_helper.c SOIL/src/SOIL.c SOIL/src/stb_image_aug.c && exit
