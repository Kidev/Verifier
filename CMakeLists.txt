cmake_minimum_required (VERSION 3.6)
project (Verifier)

find_package(OpenSSL REQUIRED)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-pedantic -Wall -Wextra -Werror -g3")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

add_library(verifier
    src/verifier.c
    src/verifier.h
)

add_executable(verify src/verify.c)
target_link_libraries(verify verifier)
