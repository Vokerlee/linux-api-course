#########################################################################

# CMAKE MINIMAL VERSION
cmake_minimum_required (VERSION 3.10)

# PROJECT PROPERTIES
project ("Shared stack" LANGUAGES C)

#########################################################################

# EXECUTABLE FILES PROPERTIES
set(TEST_PROGRAM "test_program")

set(ERRORS_SRC
        error_src/sys_err.c
)

set(PROG_SRC
        stack_src/main.c
        stack_src/stack.c
)    

# EXECUTABLE FILES
add_executable(${TEST_PROGRAM} ${PROG_SRC} ${ERRORS_SRC})

target_include_directories(${TEST_PROGRAM} PRIVATE error_src)
target_include_directories(${TEST_PROGRAM} PRIVATE include)


#########################################################################

# ASSEMBLY OPTIONS (FOR BOTH FILES)
add_compile_options(
    -Werror

    -Wall
    -Wextra
    -Wpedantic
)

#########################################################################
