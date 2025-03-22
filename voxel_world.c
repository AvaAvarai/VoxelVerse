#include "voxel_world.h"
#include <stdlib.h>
#include <math.h>

// Simple noise function for terrain generation
float noise2d(float x, float z) {
    return (float)rand() / RAND_MAX;
}

void init_voxel_world(VoxelWorld* world) {
    world->player_chunk_x = VIEW_DISTANCE;
    world->player_chunk_z = VIEW_DISTANCE;
    world->world_offset_x = 0;
    world->world_offset_z = 0;
    
    // Initialize all chunks as unloaded
    for (int x = 0; x < CHUNK_COUNT; x++) {
        for (int z = 0; z < CHUNK_COUNT; z++) {
            world->chunks[x][z].is_loaded = false;
            world->chunks[x][z].world_x = x - VIEW_DISTANCE;
            world->chunks[x][z].world_z = z - VIEW_DISTANCE;
        }
    }
    
    // Generate initial chunks around player in a 360-degree radius
    for (int x = 0; x < CHUNK_COUNT; x++) {
        for (int z = 0; z < CHUNK_COUNT; z++) {
            generate_chunk_terrain(&world->chunks[x][z]);
            world->chunks[x][z].is_loaded = true;
        }
    }
}

void update_chunks(VoxelWorld* world, float player_x, float player_z) {
    // Convert player position to chunk coordinates
    int new_chunk_x = (int)floorf(player_x / CHUNK_SIZE);
    int new_chunk_z = (int)floorf(player_z / CHUNK_SIZE);
    
    // Check if player has moved to a new chunk
    if (new_chunk_x != world->player_chunk_x || new_chunk_z != world->player_chunk_z) {
        // Calculate how many chunks to shift
        int dx = new_chunk_x - world->player_chunk_x;
        int dz = new_chunk_z - world->player_chunk_z;
        
        // Shift chunks array
        if (dx != 0 || dz != 0) {
            // Create temporary array for new chunks
            Chunk new_chunks[CHUNK_COUNT][CHUNK_COUNT];
            
            // Initialize new chunks as unloaded
            for (int x = 0; x < CHUNK_COUNT; x++) {
                for (int z = 0; z < CHUNK_COUNT; z++) {
                    new_chunks[x][z].is_loaded = false;
                    new_chunks[x][z].world_x = world->world_offset_x + (x - VIEW_DISTANCE);
                    new_chunks[x][z].world_z = world->world_offset_z + (z - VIEW_DISTANCE);
                }
            }
            
            // Copy existing chunks to their new positions
            for (int x = 0; x < CHUNK_COUNT; x++) {
                for (int z = 0; z < CHUNK_COUNT; z++) {
                    int old_x = x - dx;
                    int old_z = z - dz;
                    
                    if (old_x >= 0 && old_x < CHUNK_COUNT && 
                        old_z >= 0 && old_z < CHUNK_COUNT) {
                        new_chunks[x][z] = world->chunks[old_x][old_z];
                    }
                }
            }
            
            // Generate new chunks that need to be loaded
            for (int x = 0; x < CHUNK_COUNT; x++) {
                for (int z = 0; z < CHUNK_COUNT; z++) {
                    if (!new_chunks[x][z].is_loaded) {
                        generate_chunk_terrain(&new_chunks[x][z]);
                        new_chunks[x][z].is_loaded = true;
                    }
                }
            }
            
            // Update world state
            for (int x = 0; x < CHUNK_COUNT; x++) {
                for (int z = 0; z < CHUNK_COUNT; z++) {
                    world->chunks[x][z] = new_chunks[x][z];
                }
            }
            
            world->player_chunk_x = new_chunk_x;
            world->player_chunk_z = new_chunk_z;
            world->world_offset_x += dx;
            world->world_offset_z += dz;
        }
    }
}

void generate_chunk_terrain(Chunk* chunk) {
    // Simple terrain generation with smoother transitions
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            // Calculate world coordinates for this block
            float world_x = (chunk->world_x * CHUNK_SIZE + x) * 0.05f;  // Reduced frequency for smoother terrain
            float world_z = (chunk->world_z * CHUNK_SIZE + z) * 0.05f;
            
            // Use noise for height variation with smoother transitions
            float height_noise = noise2d(world_x, world_z);
            int ground_height = WORLD_HEIGHT / 2 + (int)(height_noise * 8); // Increased height variation
            
            // Ensure ground height stays within bounds
            if (ground_height < 0) ground_height = 0;
            if (ground_height >= WORLD_HEIGHT) ground_height = WORLD_HEIGHT - 1;
            
            // Fill blocks from bottom to ground
            for (int y = 0; y < WORLD_HEIGHT; y++) {
                if (y < ground_height) {
                    chunk->blocks[x][y][z] = 1; // Dirt
                } else if (y == ground_height) {
                    chunk->blocks[x][y][z] = 2; // Grass
                } else {
                    chunk->blocks[x][y][z] = 0; // Air
                }
            }
        }
    }
}

void render_chunk(Chunk* chunk) {
    if (!chunk->is_loaded) return;
    
    glBegin(GL_QUADS);
    
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (chunk->blocks[x][y][z] == 0) continue; // Skip air blocks
                
                // Set color based on block type
                switch (chunk->blocks[x][y][z]) {
                    case 1: // Dirt
                        glColor3f(0.6f, 0.3f, 0.0f);
                        break;
                    case 2: // Grass
                        glColor3f(0.0f, 0.8f, 0.0f);
                        break;
                }
                
                float x0 = x;
                float x1 = x + 1.0f;
                float y0 = y;
                float y1 = y + 1.0f;
                float z0 = z;
                float z1 = z + 1.0f;
                
                // Front face
                glColor3f(0.8f, 0.4f, 0.0f); // Slightly different colors for each face
                glVertex3f(x0, y0, z0);
                glVertex3f(x1, y0, z0);
                glVertex3f(x1, y1, z0);
                glVertex3f(x0, y1, z0);
                
                // Back face
                glColor3f(0.7f, 0.35f, 0.0f);
                glVertex3f(x0, y0, z1);
                glVertex3f(x0, y1, z1);
                glVertex3f(x1, y1, z1);
                glVertex3f(x1, y0, z1);
                
                // Top face
                glColor3f(0.0f, 0.8f, 0.0f);
                glVertex3f(x0, y1, z0);
                glVertex3f(x1, y1, z0);
                glVertex3f(x1, y1, z1);
                glVertex3f(x0, y1, z1);
                
                // Bottom face
                glColor3f(0.5f, 0.25f, 0.0f);
                glVertex3f(x0, y0, z0);
                glVertex3f(x0, y0, z1);
                glVertex3f(x1, y0, z1);
                glVertex3f(x1, y0, z0);
                
                // Right face
                glColor3f(0.65f, 0.325f, 0.0f);
                glVertex3f(x1, y0, z0);
                glVertex3f(x1, y0, z1);
                glVertex3f(x1, y1, z1);
                glVertex3f(x1, y1, z0);
                
                // Left face
                glColor3f(0.75f, 0.375f, 0.0f);
                glVertex3f(x0, y0, z0);
                glVertex3f(x0, y1, z0);
                glVertex3f(x0, y1, z1);
                glVertex3f(x0, y0, z1);
            }
        }
    }
    
    glEnd();
}

unsigned char get_block(VoxelWorld* world, int x, int y, int z) {
    int chunk_x = x / CHUNK_SIZE - world->world_offset_x + VIEW_DISTANCE;
    int chunk_z = z / CHUNK_SIZE - world->world_offset_z + VIEW_DISTANCE;
    
    if (chunk_x < 0 || chunk_x >= CHUNK_COUNT || 
        chunk_z < 0 || chunk_z >= CHUNK_COUNT ||
        y < 0 || y >= WORLD_HEIGHT) {
        return 0; // Air outside world bounds
    }
    
    int local_x = x % CHUNK_SIZE;
    int local_z = z % CHUNK_SIZE;
    
    return world->chunks[chunk_x][chunk_z].blocks[local_x][y][local_z];
}

void set_block(VoxelWorld* world, int x, int y, int z, unsigned char block_type) {
    int chunk_x = x / CHUNK_SIZE - world->world_offset_x + VIEW_DISTANCE;
    int chunk_z = z / CHUNK_SIZE - world->world_offset_z + VIEW_DISTANCE;
    
    if (chunk_x < 0 || chunk_x >= CHUNK_COUNT || 
        chunk_z < 0 || chunk_z >= CHUNK_COUNT ||
        y < 0 || y >= WORLD_HEIGHT) {
        return;
    }
    
    int local_x = x % CHUNK_SIZE;
    int local_z = z % CHUNK_SIZE;
    
    world->chunks[chunk_x][chunk_z].blocks[local_x][y][local_z] = block_type;
}

void cleanup_voxel_world(VoxelWorld* world) {
    // Nothing to clean up in this simple implementation
} 