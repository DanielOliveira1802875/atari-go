cmake_minimum_required(VERSION 3.10)
project(AtariGoProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set optimization level for all build types if not explicitly set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Common compiler flags based on compiler
if(MSVC)
    # MSVC compiler optimizations
    add_compile_options(/MP /O2 /Ob3 /Oi /Ot /GL /fp:fast /arch:AVX2)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
else()
    # GCC/Clang optimizations
    add_compile_options(
            -O3
            -march=native
            -ffast-math
            -funroll-loops
            -ftree-vectorize
    )

    # Enable OpenMP if available
    find_package(OpenMP QUIET)
    if(OpenMP_CXX_FOUND)
        add_compile_options(${OpenMP_CXX_FLAGS})
    endif()
endif()

# Enable LTO/IPO
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED)
if(LTO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Required packages
find_package(Threads REQUIRED)

# Source files
add_executable(
        atari_go
        main.cpp
        AtariGo.cpp
        Board.h
        MiniMax.cpp
        Zobrist.h
        BBUtils.h
)

# Include directories
target_include_directories(atari_go PRIVATE ${CMAKE_SOURCE_DIR})

# Link libraries
target_link_libraries(atari_go PRIVATE Threads::Threads)

# Output directories
set_target_properties(atari_go PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)