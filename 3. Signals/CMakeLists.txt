#########################################################################

# CMAKE MINIMAL VERSION
cmake_minimum_required (VERSION 3.10)

# PROJECT PROPERTIES
project ("Signals" LANGUAGES C)

#########################################################################

# EXECUTABLE FILES PROPERTIES
set(TRANSMITTER_NAME "transmitter")
set(RECIEVER_NAME    "reciever")

set(ERRORS_SRC
        error_src/sys_err.c
)

set(TRANS_SRC
        transmitter/main.c
        transmitter/trans_err.c
        transmitter/trans_handler.c
)

set(REC_SRC
        receiver/main.c
        receiver/rec_err.c
        receiver/rec_handler.c
)    

# EXECUTABLE FILES
add_executable(${TRANSMITTER_NAME} ${TRANS_SRC})
add_executable(${RECIEVER_NAME}    ${REC_SRC})

target_include_directories(${TRANSMITTER_NAME} PRIVATE error_src)
target_include_directories(${RECIEVER_NAME}    PRIVATE error_src)


#########################################################################

# ASSEMBLY OPTIONS

add_compile_options(
    -Werror

    -Wall
    -Wextra
    -Wpedantic
)

#########################################################################