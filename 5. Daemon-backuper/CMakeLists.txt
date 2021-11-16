#########################################################################

# CMAKE MINIMAL VERSION
cmake_minimum_required (VERSION 3.10)

# PROJECT PROPERTIES
project ("Backuper" LANGUAGES C)

#########################################################################

# EXECUTABLE FILES PROPERTIES
set(BACKUPER_NAME "backuperd")

set(BACKUPER_SRC
        daemon_src/main.c
        daemon_src/daemon.c
        daemon_src/args_handling.c
        daemon_src/backuper.c
)    

# EXECUTABLE FILES
add_executable(${BACKUPER_NAME} ${BACKUPER_SRC})

target_include_directories(${BACKUPER_NAME} PRIVATE include)

#########################################################################

# ASSEMBLY OPTIONS (FOR BOTH FILES)
add_compile_options(
    -Werror

    -Wall
    -Wextra
    -Wpedantic
)

#########################################################################
