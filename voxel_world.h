#ifndef VOXEL_WORLD_H
#define VOXEL_WORLD_H

#include <SDL.h>
#include <OpenGL/gl.h>
#include <stdbool.h>

#define CHUNK_SIZE 16
#define WORLD_HEIGHT 32
#define VIEW_DISTANCE 4  // Number of chunks visible in each direction
#define CHUNK_COUNT (VIEW_DISTANCE * 2 + 1)  // Total chunks in view (including center)
#define DAY_LENGTH 1200.0f  // Length of a full day cycle in seconds
#define NUM_STARS 1000  // Number of stars in the night sky

typedef struct {
    float r, g, b;
} Color;

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    Vector3 position;
    float brightness;
} Star;

typedef struct {
    Color sky_color;
    Color fog_color;
    float time_of_day;  // 0.0 to 1.0, where 0.0 is dawn
    float day_cycle_speed;
    Star stars[NUM_STARS];
    float sun_angle;  // Angle of the sun in radians
} Skybox;

typedef struct {
    unsigned char blocks[CHUNK_SIZE][WORLD_HEIGHT][CHUNK_SIZE];
    bool is_loaded;
    int world_x;  // World coordinates of this chunk
    int world_z;
} Chunk;

typedef struct {
    Chunk chunks[CHUNK_COUNT][CHUNK_COUNT];
    int player_chunk_x;  // Current chunk coordinates of player
    int player_chunk_z;
    int world_offset_x;  // World coordinates of center chunk
    int world_offset_z;
    Skybox skybox;
} VoxelWorld;

// Initialize the voxel world
void init_voxel_world(VoxelWorld* world);

// Update chunks based on player position
void update_chunks(VoxelWorld* world, float player_x, float player_z);

// Generate terrain for a chunk
void generate_chunk_terrain(Chunk* chunk);

// Render a chunk
void render_chunk(Chunk* chunk);

// Get block at world coordinates
unsigned char get_block(VoxelWorld* world, int x, int y, int z);

// Set block at world coordinates
void set_block(VoxelWorld* world, int x, int y, int z, unsigned char block_type);

// Update skybox colors based on time of day
void update_skybox(VoxelWorld* world, float delta_time);

// Render the skybox
void render_skybox(VoxelWorld* world);

// Clean up the voxel world
void cleanup_voxel_world(VoxelWorld* world);

#endif // VOXEL_WORLD_H