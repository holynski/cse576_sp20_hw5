cmake_minimum_required(VERSION 2.8)

project(cse576-hw5)

set(CMAKE_CXX_FLAGS "-fdiagnostics-color=always -std=c++11 -pthread -O2 -g -march=native -fPIC")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)


add_library(uwimg++ SHARED
     src/utils.cpp
     src/utils.h
     src/image.h
     src/load_image.cpp
     src/stb_image.h
     src/stb_image_write.h
     
     src/process_image.cpp
     src/resize_image.cpp
     src/filter_image.cpp
     
     src/harris_image.cpp
     src/panorama_image.cpp
     
     src/matrix.cpp
     src/matrix.h
     
     )

link_libraries(uwimg++ m stdc++)

add_executable(test1 src/test/test1.cpp)
add_executable(test2 src/test/test2.cpp)
add_executable(test5 src/test/test5.cpp)
add_executable(make-panorama src/test/make-panorama.cpp)

add_subdirectory(src/pango)
