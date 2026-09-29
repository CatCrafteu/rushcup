cmake_minimum_required(VERSION 3.20)
project(ViceCityLauncher VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(/W4 /WX /permissive- /Zc:inline /Zc:strictStrings)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL /LTCG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -fno-exceptions -fno-rtti)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto -s")
endif()

# Include nlohmann/json from main project
add_subdirectory(${CMAKE_SOURCE_DIR}/../vendor/nlohmann_json)

add_executable(ViceCityLauncher 
    ViceCityLauncher.cpp
    ConfigLoader.hpp
)

target_include_directories(ViceCityLauncher PRIVATE
    ${CMAKE_SOURCE_DIR}/../vendor/nlohmann_json/include
)

target_compile_definitions(ViceCityLauncher PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
    JSON_USE_IMPLICIT_CONVERSIONS=0
)

target_link_libraries(ViceCityLauncher PRIVATE
    nlohmann_json
    shell32
    advapi32
    psapi
    version
)

# Copy DLL to build directory for testing
add_custom_command(TARGET ViceCityLauncher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/../build/bin/${CMAKE_BUILD_TYPE}/ViceCity.dll"
    "${CMAKE_BINARY_DIR}/ViceCity.dll"
    COMMENT "Copying ViceCity.dll to launcher output directory"
)

# Copy config file
add_custom_command(TARGET ViceCityLauncher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/launcher_config.json"
    "${CMAKE_BINARY_DIR}/launcher_config.json"
    COMMENT "Copying launcher config"
)

# Install
install(TARGETS ViceCityLauncher
    RUNTIME DESTINATION .
)
install(FILES "${CMAKE_SOURCE_DIR}/launcher_config.json" DESTINATION .)