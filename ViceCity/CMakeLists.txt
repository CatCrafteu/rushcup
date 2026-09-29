cmake_minimum_required(VERSION 3.20)
project(ViceCity VERSION 1.0.0 LANGUAGES CXX)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "" FORCE)

# MSVC specific
if(MSVC)
    # Runtime library
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # Release optimizations
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL /Gy /Oi /Ot /Oy /Ob2 /FS")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF /DYNAMICBASE /NXCOMPAT /RELEASE /DEBUG:NONE")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF /DYNAMICBASE /NXCOMPAT /RELEASE /DEBUG:NONE")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF /DYNAMICBASE /NXCOMPAT /RELEASE /DEBUG:NONE")

    # Debug flags
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /Ob0 /RTC1 /MTd")
else()
    # GCC/Clang
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto -s")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -s -Wl,--gc-sections")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -flto -s -Wl,--gc-sections")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -flto -s -Wl,--gc-sections")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/features
    ${CMAKE_SOURCE_DIR}/gui
    ${CMAKE_SOURCE_DIR}/resolver
    ${CMAKE_SOURCE_DIR}/skinchanger_system
    ${CMAKE_SOURCE_DIR}/vendor
)

# Vendor dependencies
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/minhook)
add_subdirectory(vendor/nlohmann_json)
add_subdirectory(vendor/lua)
add_subdirectory(vendor/fmt)

# Main target - glob all cpp files (excluding vendor, launcher, build)
file(GLOB VICECITY_SOURCES
    ${CMAKE_SOURCE_DIR}/core/*.cpp
    ${CMAKE_SOURCE_DIR}/core/*/*.cpp
    ${CMAKE_SOURCE_DIR}/features/*.cpp
    ${CMAKE_SOURCE_DIR}/features/*/*.cpp
    ${CMAKE_SOURCE_DIR}/gui/*.cpp
    ${CMAKE_SOURCE_DIR}/gui/*/*.cpp
    ${CMAKE_SOURCE_DIR}/resolver/*.cpp
    ${CMAKE_SOURCE_DIR}/skinchanger_system/*.cpp
)

add_library(ViceCity SHARED ${VICECITY_SOURCES})

target_link_libraries(ViceCity PRIVATE
    imgui
    minhook
    nlohmann_json
    lua
    fmt
    d3d11
    dxgi
    dxguid
    dwrite
    windowscodecs
    dwmapi
    psapi
    version
)

target_compile_definitions(ViceCity PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
    IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
    IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
    SOL_ALL_SAFETIES_ON=1
    SOL_USE_BOOST=0
    SOL_USING_BOOST=0
    FMT_HEADER_ONLY
)

# Precompiled header
target_include_directories(ViceCity PRIVATE
    ${CMAKE_SOURCE_DIR}/vendor/sol2/include
    ${CMAKE_SOURCE_DIR}/vendor/lua
)

# Install
install(TARGETS ViceCity
    LIBRARY DESTINATION bin
    RUNTIME DESTINATION bin
)

# Custom targets
add_custom_target(ViceCity_Clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/lib
    COMMENT "Cleaning build output"
)

# Package
set(CPACK_PACKAGE_NAME "ViceCity")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION "ViceCity CS2 Cheat")
set(CPACK_GENERATOR "ZIP")
include(CPack)

# Print config
message(STATUS "ViceCity Build Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Output Dir: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")