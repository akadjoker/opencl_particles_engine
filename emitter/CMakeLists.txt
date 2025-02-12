
project(emitter)
cmake_policy(SET CMP0072 NEW)
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")





set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_compile_options(
    -Wall 
    -Wextra
    -Werror
 
)

file(GLOB SOURCES "src/*.cpp")
add_executable(emitter  ${SOURCES})
target_include_directories(emitter PUBLIC  include  src)

if(CMAKE_BUILD_TYPE MATCHES Debug)

    target_compile_options(emitter PRIVATE    -fno-omit-frame-pointer -g  -D_DEBUG )
    target_link_options(emitter PRIVATE     -D_DEBUG) 

       # target_compile_options(emitter PRIVATE -fsanitize=address -fsanitize=undefined -fsanitize=leak   -fno-omit-frame-pointer -g  -D_DEBUG )
       # target_link_options(emitter PRIVATE -fsanitize=address -fsanitize=undefined -fsanitize=leak    -D_DEBUG) 



elseif(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(emitter PRIVATE -O3   -DNDEBUG )
    target_link_options(emitter PRIVATE -O3   -DNDEBUG )
endif()

find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
 

    target_link_libraries(emitter
    PRIVATE
        m
        OpenGL::GL
        OpenCL::OpenCL
        GLEW 
        glfw
)
