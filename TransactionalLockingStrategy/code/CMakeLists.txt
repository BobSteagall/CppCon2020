cmake_minimum_required (VERSION 3.5)
project (tx_test)

include_directories(src)

set(Sources
    src/forward.hpp
    src/lockable_item.cpp
    src/lockable_item.hpp
    src/transaction_manager.cpp
    src/transaction_manager.hpp

    src/tests.cpp
    src/main.cpp
)

add_executable(tx_test ${Sources})
target_link_libraries(tx_test pthread)

set(CMAKE_VERBOSE_MAKEFILE 1)

if(CXX_COMPILER STREQUAL clang++)
    set(CMAKE_C_COMPILER   clang)
    set(CMAKE_CXX_COMPILER clang++)
else()
    set(CMAKE_C_COMPILER   gcc)
    set(CMAKE_CXX_COMPILER g++)
endif()

if(CMAKE_CXX_COMPILER STREQUAL clang++)

    set(CMAKE_C_FLAGS_DEBUG   "-g -Wall -pedantic -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++17 -Wall -pedantic -Wextra -Wno-unused-parameter")

    set(CMAKE_C_FLAGS_RELEASE   "-O3 -march=skylake -Wall -pedantic -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=skylake -std=c++17 -stdlib=libc++ -Wall -pedantic -Wextra -Wno-unused-parameter")

elseif(CMAKE_CXX_COMPILER STREQUAL g++)

    set(CMAKE_C_FLAGS_DEBUG   "-g -Wall -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++17 -Wall -pedantic -Wextra")

    set(CMAKE_C_FLAGS_RELEASE   "-O2 -march=skylake -Wall -pedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -march=skylake -std=c++17 -Wall -pedantic -Wextra")


    set(CMAKE_C_FLAGS_TSAN   "-fsanitize=thread -g -O2 -march=skylake -Wall -pedantic")
    set(CMAKE_CXX_FLAGS_TSAN "-fsanitize=thread -g -O2 -march=skylake -std=c++17 -Wall -pedantic -Wextra")
endif()
