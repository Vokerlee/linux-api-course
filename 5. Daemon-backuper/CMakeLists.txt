#########################################################################

# CMAKE MINIMAL VERSION
cmake_minimum_required (VERSION 3.10)

# PROJECT PROPERTIES
project ("Backuper" LANGUAGES C)

#########################################################################

# ASSEMBLY OPTIONS
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
set(BACKUPER_NAME     "backuperd")
set(BACKUPER_CTL_NAME "backuperd_ctl")

set(BACKUPER_SRC
    daemon_src/main.c
    daemon_src/daemon.c
    daemon_src/args_handling.c
    daemon_src/backuper.c
    hash_table/hash_table.c
)   

set(BACKUPER_CTL_SRC
    daemon_ctl/backuperd_ctl.c
)  

# EXECUTABLE FILES
add_executable(${BACKUPER_NAME}     ${BACKUPER_SRC})
add_executable(${BACKUPER_CTL_NAME} ${BACKUPER_CTL_SRC})

target_include_directories(${BACKUPER_NAME} PRIVATE include)
target_include_directories(${BACKUPER_NAME} PRIVATE hash_table)

#########################################################################
