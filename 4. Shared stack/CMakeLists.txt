#########################################################################

# CMAKE MINIMAL VERSION
cmake_minimum_required (VERSION 3.10)

# PROJECT PROPERTIES
project ("Shared stack" LANGUAGES C)

#########################################################################

# ASSEMBLY OPTIONS (FOR BOTH FILES)
add_compile_options(
    -Wall

    -Wno-pointer-to-int-cast
    -Wno-int-to-pointer-cast
    -Wno-variadic-macros
    -Wno-unused-variable
    -Wno-unused-parameter
    -Wno-unused-function
    -Wno-sign-compare
    -Wno-misleading-indentation
    -Wno-format
)

#########################################################################

# EXECUTABLE FILES PROPERTIES
set(TEST_PROGRAM1 "test_program1")
set(TEST_PROGRAM2 "test_program2")

set(ERRORS_SRC
    error_src/sys_err.c
)

set(PROG_SRC1
    stack_src/main1.c
    stack_src/stack.c
) 

set(PROG_SRC2
    stack_src/main2.c
    stack_src/stack.c
)    

# EXECUTABLE FILES
add_executable(${TEST_PROGRAM1} ${PROG_SRC1} ${ERRORS_SRC})
add_executable(${TEST_PROGRAM2} ${PROG_SRC2} ${ERRORS_SRC})

target_include_directories(${TEST_PROGRAM1} PRIVATE error_src)
target_include_directories(${TEST_PROGRAM1} PRIVATE include)
target_include_directories(${TEST_PROGRAM2} PRIVATE error_src)
target_include_directories(${TEST_PROGRAM2} PRIVATE include)

#########################################################################
