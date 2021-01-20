cmake_minimum_required(VERSION 3.15...3.19 FATAL_ERROR)

project(eventio VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

# IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_library(eventio SHARED src/File.cpp src/Object.cpp)
set_property(TARGET eventio PROPERTY PUBLIC_HEADER include/eventio/File.h;include/eventio/Object.h)
target_include_directories(eventio PUBLIC include)

target_compile_features(eventio PUBLIC cxx_std_17)
set_property(TARGET eventio PROPERTY CXX_EXTENSIONS OFF)
target_compile_options(eventio PRIVATE -Wall -Wextra -pedantic)

add_executable(read_eventio cli/read_eventio.cpp)
target_compile_features(read_eventio PRIVATE cxx_std_17)
set_property(TARGET read_eventio PROPERTY CXX_EXTENSIONS OFF)
target_compile_options(read_eventio PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(read_eventio eventio)


include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
