project(opengl-lighting-test)
add_executable(opengl-lighting-test src/main.cpp src/line.cpp)
target_link_libraries(opengl-lighting-test GL GLEW glfw)
