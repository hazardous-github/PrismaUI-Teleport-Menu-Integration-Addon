cmake_minimum_required(VERSION 3.24)

set(PTMI_VERSION "0.1.0" CACHE STRING "")

project(
    PrismaUITeleportMenuIntegration
    VERSION ${PTMI_VERSION}
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_SE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR ON CACHE BOOL "" FORCE)
set(COMMONLIB_PREBUILT ON CACHE BOOL "" FORCE)
set(COMMONLIB_PREBUILT_MULTICONFIG ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    CommonLibSSE
    GIT_REPOSITORY https://github.com/alandtse/CommonLibSSE-NG.git
    GIT_TAG v6.1.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(CommonLibSSE)

include("${commonlibsse_SOURCE_DIR}/cmake/CommonLibSSE.cmake")

add_commonlibsse_plugin(${PROJECT_NAME}
    SOURCES
        src/main.cpp
        src/PCH.h
        src/SKSEMenuFrameworkAdapter.h
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_precompile_headers(${PROJECT_NAME}
    PRIVATE
        src/PCH.h
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
endif()
