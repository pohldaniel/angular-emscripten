emcc -c -s DISABLE_EXCEPTION_CATCHING=0 -std=c11 -Wall -Wextra -Wpedantic -Oz libsoil/image_DXT.c libsoil/image_helper.c libsoil/SOIL.c libsoil/stb_image_aug.c && exit
