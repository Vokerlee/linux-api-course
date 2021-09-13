#########################################################################

# CMAKE MINIMAL VERSION
cmake_minimum_required (VERSION 3.8)

# PROJECT PROPERTIES
project ("BIZZ_BUZZ" LANGUAGES C)

#########################################################################

# EXECUTABLE FILE PROPERTIES
set(FILE_NAME "bizz")

set(HEADERS
        errors_vh.h
        boz_handler.h
)

set(SOURCES
        main.c
        errors_vh.c
        biz_handler.c
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

    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wctor-dtor-privacy
    -Wenum-compare
    -Wfloat-equal
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    -Wsign-conversion
    -Wsign-promo
)

#########################################################################