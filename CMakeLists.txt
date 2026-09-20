cmake_minimum_required(VERSION 3.20)

project(haze
    VERSION 0.1.0
    DESCRIPTION "idk lol"
    LANGUAGES C
)

# ============================================================
# Haze
# ============================================================
#
# Haze is a C11 project.
#
# Requirements:
#   - CMake 3.20+
#   - C11-compatible compiler
#   - Git
#   - Internet connection during configuration
#
# Tested platforms:
#   - Linux: GCC, Clang
#   - Windows: MinGW
#
# macOS:
#   - Expected to work
#   - Not officially tested yet
#
# Build:
#
#   cmake -S . -B build
#   cmake --build build
#
# Linux / macOS:
#
#   ./build/haze
#
# Windows / MinGW:
#
#   .\build\haze.exe
#
# The Haze core does not require C++ or Go.
#
# libuv is downloaded automatically by CMake through FetchContent.
# Dependencies inside third_party/ are compiled automatically.
#
# ============================================================


# ============================================================
# Language
# ============================================================

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# ============================================================
# Paths
# ============================================================

set(HAZE_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/source"
)

set(HAZE_THIRD_PARTY_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
)


# ============================================================
# Dependencies
# ============================================================

include(FetchContent)

#
# libuv
#
# Used by the Haze server for cross-platform asynchronous I/O.
#
# The dependency is downloaded automatically during CMake
# configuration. No manual installation is required.
#

FetchContent_Declare(
    libuv
    GIT_REPOSITORY https://github.com/libuv/libuv.git
    GIT_TAG        v1.51.0
)

FetchContent_MakeAvailable(libuv)


# ============================================================
# Helpers
# ============================================================

#
# Recursively collect C sources.
#
# This allows the source tree to be organized by module instead
# of requiring every source file to live directly inside src/.
#

function(haze_collect_sources OUTPUT DIRECTORY)

    file(GLOB_RECURSE SOURCES
        CONFIGURE_DEPENDS
        "${DIRECTORY}/*.c"
    )

    set(${OUTPUT}
        ${SOURCES}
        PARENT_SCOPE
    )

endfunction()


# ============================================================
# Third-party sources
# ============================================================

if(EXISTS "${HAZE_THIRD_PARTY_DIR}")

    haze_collect_sources(
        HAZE_THIRD_PARTY_SOURCES
        "${HAZE_THIRD_PARTY_DIR}"
    )

    if(HAZE_THIRD_PARTY_SOURCES)

        add_library(
            haze_third_party
            STATIC
            ${HAZE_THIRD_PARTY_SOURCES}
        )

        target_include_directories(
            haze_third_party
            PUBLIC
            "${HAZE_THIRD_PARTY_DIR}"
        )

    endif()

endif()


# ============================================================
# Core sources
# ============================================================

haze_collect_sources(
    HAZE_CORE_SOURCES
    "${HAZE_SOURCE_DIR}"
)

if(NOT HAZE_CORE_SOURCES)

    message(
        FATAL_ERROR
        "No C source files found in ${HAZE_SOURCE_DIR}"
    )

endif()


# ============================================================
# Haze Core Library
# ============================================================

#
# The core library contains the Haze engine without main().
#
# This allows other Haze components and clients to link against
# the core without creating a second executable entry point.
#

set(HAZE_LIB_SOURCES
    ${HAZE_CORE_SOURCES}
)

list(
    FILTER HAZE_LIB_SOURCES
    EXCLUDE
    REGEX ".*main\\.c$"
)

add_library(
    hazecore
    STATIC
    ${HAZE_LIB_SOURCES}
)

target_include_directories(
    hazecore
    PUBLIC
    "${HAZE_SOURCE_DIR}"
)

target_link_libraries(
    hazecore
    PUBLIC
    uv
)

if(TARGET haze_third_party)

    target_link_libraries(
        hazecore
        PUBLIC
        haze_third_party
    )

endif()


# ============================================================
# Haze Executable
# ============================================================

add_executable(
    hazec
    ${HAZE_CORE_SOURCES}
)

target_include_directories(
    hazec
    PUBLIC
    "${HAZE_SOURCE_DIR}"
)

target_link_libraries(
    hazec
    PUBLIC
    uv
)

if(WIN32)
    add_custom_command(
        TARGET hazec
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:uv>"
            "$<TARGET_FILE_DIR:hazec>"
    )
endif()

if(TARGET haze_third_party)

    target_link_libraries(
        hazec
        PRIVATE
        haze_third_party
    )

endif()


# ============================================================
# Platform configuration
# ============================================================

#
# Windows
#
# Haze is a console application.
#

if(WIN32)

    set_target_properties(
        hazec
        PROPERTIES
        WIN32_EXECUTABLE FALSE
    )

endif()


#
# Unix-like systems
#
# _XOPEN_SOURCE enables the POSIX interfaces used by the core.
#

if(UNIX AND NOT APPLE)

    target_compile_definitions(
        hazecore
        PRIVATE
        _XOPEN_SOURCE=700
    )

    target_compile_definitions(
        hazec
        PRIVATE
        _XOPEN_SOURCE=700
    )

endif()


# ============================================================
# Compiler warnings
# ============================================================

if(MSVC)

    target_compile_options(
        hazecore
        PRIVATE
        /W4
    )

    target_compile_options(
        hazec
        PRIVATE
        /W4
    )

else()

    target_compile_options(
        hazecore
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    target_compile_options(
        hazec
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

endif()


# ============================================================
# Build information
# ============================================================

message(STATUS "")
message(STATUS "Haze ${PROJECT_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C standard: C11")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")


# ============================================================
# Installation
# ============================================================

#
# Optional installation support:
#
#   cmake --install build
#
# Installs the Haze executable into the platform's standard
# CMake binary installation directory.
#

include(GNUInstallDirs)

install(
    TARGETS hazec
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

enable_testing()
add_subdirectory(tests/unit)
