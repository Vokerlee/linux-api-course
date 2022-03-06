#########################################################################

# CMAKE MINIMAL VERSION
cmake_minimum_required (VERSION 3.8)

# PROJECT PROPERTIES
project ("BIZZ_BUZZ" LANGUAGES C)

#########################################################################

# ASSEMBLY OPTIONS

add_compile_options(
    -Wall

    -Wno-pointer-to-int-cast
    -Wno-int-to-pointer-cast
    -Wno-variadic-macros
    -Wno-unused-parameter
    -Wno-unused-function
    -Wno-sign-compare
    -Wno-misleading-indentation
)

#########################################################################

# EXECUTABLE FILE PROPERTIES
set(FILE_NAME "bizz")

set(HEADERS
    src/biz_err.h
    src/biz_handler.h
)

set(SOURCES
    src/main.c
    src/biz_err.c
    src/sys_err.c
    src/biz_handler.c
)

# EXECUTABLE FILE
add_executable(${FILE_NAME} ${SOURCES})

#########################################################################
