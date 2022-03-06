#########################################################################

# CMAKE MINIMAL VERSION
cmake_minimum_required (VERSION 3.10)

# PROJECT PROPERTIES
project ("Signals" LANGUAGES C)

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
set(TRANSMITTER_NAME "transmitter")
set(RECIEVER_NAME    "receiver")

set(GENERAL_SRC
    error_src/sys_err.c
    interaction/data_manip.c
)

set(TRANS_SRC
    transmitter/trans_main.c
    transmitter/trans_err.c
    transmitter/trans_handler.c
)

set(REC_SRC
    receiver/rec_main.c
    receiver/rec_err.c
    receiver/rec_handler.c
)    

# EXECUTABLE FILES
add_executable(${TRANSMITTER_NAME} ${TRANS_SRC} ${GENERAL_SRC})
add_executable(${RECIEVER_NAME}    ${REC_SRC}   ${GENERAL_SRC})

# ERRORS FUNCTIiONS (GENERAL PURPOSE)
target_include_directories(${TRANSMITTER_NAME} PRIVATE error_src)
target_include_directories(${RECIEVER_NAME}    PRIVATE error_src)
target_include_directories(${TRANSMITTER_NAME} PRIVATE interaction)
target_include_directories(${RECIEVER_NAME}    PRIVATE interaction)

#########################################################################
