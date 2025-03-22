cmake_minimum_required(VERSION 3.10)
project(voxel_game)

# Set SDL2 paths
set(SDL2_INCLUDE_DIRS "/opt/homebrew/include/SDL2")
set(SDL2_LIBRARIES "/opt/homebrew/lib/libSDL2.dylib")

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add executable
add_executable(voxel_game main.c voxel_world.c)

# Include directories
target_include_directories(voxel_game PRIVATE 
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Headers
)

# Link libraries
target_link_libraries(voxel_game PRIVATE 
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    "-framework OpenGL"
    "-framework GLUT"
) 