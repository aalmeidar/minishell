cmake_minimum_required(VERSION 3.22)
project(minishell C)

set(CMAKE_C_STANDARD 11)

file(GLOB minishell_SRC CONFIGURE_DEPENDS "*.h" "*.c")
add_executable(minishell ${minishell_SRC})
