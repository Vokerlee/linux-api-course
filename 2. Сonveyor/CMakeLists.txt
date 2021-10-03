#########################################################################

# CMAKE MINIMAL VERSION
cmake_minimum_required (VERSION 3.8)

# PROJECT PROPERTIES
project ("Conveyor" LANGUAGES C)

#########################################################################

# EXECUTABLE FILE PROPERTIES
set(FILE_NAME "convey")

set(HEADERS
        src/conv_err.h
        src/conv_handler.h
)

set(SOURCES
        src/main.c
        src/conv_err.c
        src/sys_err.c
        src/conv_handler.c
)

# EXECUTABLE FILE
add_executable(${FILE_NAME} ${SOURCES})

#########################################################################

# ASSEMBLY OPTIONS

add_compile_options(
    -Werror

    -Wall
    -Wextra
    -Wpedantic
)

#########################################################################