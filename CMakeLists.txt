cmake_minimum_required(VERSION 3.20)
project(converter CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CONAN_DISABLE_CHECK_COMPILER "1")

set(SOURCES
    source.cpp
)

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif()

set(modules "${CMAKE_SOURCE_DIR}/cmake/Modules")
file(MAKE_DIRECTORY ${modules})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${modules}")
file(COPY ${CMAKE_BINARY_DIR}/Findnlohmann_json.cmake DESTINATION ${modules})

find_package(nlohmann_json REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

# Windows10 x64 build
# conan install -pr ./conan_profile -if build . -s build_type=Release -s compiler="Visual Studio" -s compiler.runtime=MDd --build=missing
# cmake -Bbuild . -G "Visual Studio 17" -A x64
# cmake --build ./build