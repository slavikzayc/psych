cmake_minimum_required(VERSION 3.16)

project(PSYCHEMPATHY LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

file(GLOB_RECURSE PSYCHEMPATHY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(psychempathy ${PSYCHEMPATHY_SOURCES})

target_include_directories(psychempathy PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(psychempathy PRIVATE nlohmann_json::nlohmann_json)
target_compile_definitions(psychempathy PRIVATE NOMINMAX)

if(MSVC)
    target_compile_options(psychempathy PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(psychempathy PRIVATE -Wall -Wextra -pedantic)
endif()

add_custom_command(TARGET psychempathy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:psychempathy>/assets"
)
