cmake_minimum_required(VERSION 3.30)
project(dns VERSION 0.1.0 LANGUAGES C)
add_executable(dns main.c dns.c utils.c)
target_compile_options(dns PRIVATE -Wall -Wextra)
