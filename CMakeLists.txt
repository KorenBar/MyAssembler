cmake_minimum_required(VERSION 3.19)
project(MyAssembler C)

set(CMAKE_C_STANDARD 90)

add_compile_options(-g -ansi -Wall -pedantic)
add_executable(MyAssembler main.c asmencoder.c asmencoder.h asmfile.c asmfile.h buffer.c buffer.h strhelper.c strhelper.h statement.c statement.h directive.c directive.h mytypes.c mytypes.h command.c command.h parsing.c parsing.h callbacks.c callbacks.h memohelper.c memohelper.h)