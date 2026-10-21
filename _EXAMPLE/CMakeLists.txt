cmake_minimum_required(VERSION 3.30...4.4)
project(EXAMPLE)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
	"EXAMPLE.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs")  # (for header-only libs)

set(GWINDOWER_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
add_subdirectory("libs/libGWindower")

set(GRENDERER3D_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
add_subdirectory("libs/libGRenderer3D")

add_library(LIBGGIZMO3D_EXAMPLE_glad
        "libs/glad/glad.c"
)

set(GLINERENDERER3D_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
add_subdirectory("libs/libGLineRenderer3D")

target_link_libraries(${PROJECT_NAME} PRIVATE
        GRenderer3D
        GWindower
        LIBGGIZMO3D_EXAMPLE_glad
        GLineRenderer3D
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Cube.obj" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Cube.obj" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")