# cmake -DIN=<file> -DOUT=<file.c> -DNAME=<symbol> -P embed.cmake
# Writes a C file with the bytes of IN as `const unsigned char NAME[]` and `NAME_len`.
file(READ "${IN}" hex HEX)
string(LENGTH "${hex}" n)
math(EXPR len "${n} / 2")
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," body "${hex}")
string(REGEX REPLACE "((0x..,){16})" "\\1\n" body "${body}")
file(WRITE "${OUT}" "/* Generated from ${IN} by embed.cmake. */\n#include <stddef.h>\nconst unsigned char ${NAME}[] = {\n${body}\n};\nconst size_t ${NAME}_len = ${len};\n")
