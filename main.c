#include <SDL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "voxel_world.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MOVEMENT_SPEED 0.1f
#define MOUSE_SENSITIVITY 0.2f
#define MOUSE_MOVE_SPEED 0.5f

typedef struct {
    float x, y, z;
    float pitch, yaw;
} Camera;

void init_gl() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Enable lighting for 3D effect
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Set up light position and properties
    float light_pos[] = {1.0f, 1.0f, 1.0f, 0.0f};
    float light_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    float light_diffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    
    // Enable color material for simpler coloring
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void setup_camera(Camera* camera) {
    glLoadIdentity();
    glRotatef(-camera->pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-camera->yaw, 0.0f, 1.0f, 0.0f);
    glTranslatef(-camera->x, -camera->y, -camera->z);
}

void move_camera(Camera* camera, float forward, float right) {
    float angle = camera->yaw * M_PI / 180.0f;
    camera->x += sinf(angle) * forward * MOVEMENT_SPEED;
    camera->z += cosf(angle) * forward * MOVEMENT_SPEED;
    camera->x += sinf(angle + M_PI/2) * right * MOVEMENT_SPEED;
    camera->z += cosf(angle + M_PI/2) * right * MOVEMENT_SPEED;
}

void move_camera_forward(Camera* camera) {
    float angle = camera->yaw * M_PI / 180.0f;
    camera->x += sinf(angle) * MOUSE_MOVE_SPEED;
    camera->z += cosf(angle) * MOUSE_MOVE_SPEED;
}

void move_camera_backward(Camera* camera) {
    float angle = camera->yaw * M_PI / 180.0f;
    camera->x -= sinf(angle) * MOUSE_MOVE_SPEED;
    camera->z -= cosf(angle) * MOUSE_MOVE_SPEED;
}

int main(int argc, char* argv[]) {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        printf("SDL Initialization error: %s\n", SDL_GetError());
        return 1;
    }

    // Create window with OpenGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    
    SDL_Window* window = SDL_CreateWindow("Voxel Game",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    
    if (!window) {
        printf("Window creation error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        printf("OpenGL context creation error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize OpenGL
    init_gl();
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);

    // Initialize voxel world
    VoxelWorld world;
    init_voxel_world(&world);

    // Initialize camera
    Camera camera = {
        .x = CHUNK_SIZE * CHUNK_COUNT / 2,
        .y = WORLD_HEIGHT + 10,  // Position camera higher up
        .z = CHUNK_SIZE * CHUNK_COUNT / 2,
        .pitch = -30.0f,  // Look down at a better angle
        .yaw = 0.0f
    };

    // Main loop variables
    bool quit = false;
    SDL_Event event;
    bool first_mouse = true;
    int last_mouse_x = 0, last_mouse_y = 0;
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
    bool left_mouse_down = false;
    bool right_mouse_down = false;

    // Capture mouse
    SDL_SetRelativeMouseMode(SDL_TRUE);

    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    right_mouse_down = true;
                }
                else if (event.button.button == SDL_BUTTON_RIGHT) {
                    left_mouse_down = true;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    right_mouse_down = false;
                }
                else if (event.button.button == SDL_BUTTON_RIGHT) {
                    left_mouse_down = false;
                }
            }
            else if (event.type == SDL_MOUSEMOTION) {
                if (first_mouse) {
                    last_mouse_x = event.motion.x;
                    last_mouse_y = event.motion.y;
                    first_mouse = false;
                    continue;
                }

                float xoffset = event.motion.xrel * MOUSE_SENSITIVITY;
                float yoffset = event.motion.yrel * MOUSE_SENSITIVITY;

                camera.yaw += xoffset;
                camera.pitch += yoffset;

                // Constrain pitch
                if (camera.pitch > 89.0f)
                    camera.pitch = 89.0f;
                if (camera.pitch < -89.0f)
                    camera.pitch = -89.0f;
            }
        }

        // Handle keyboard input
        float forward = 0.0f, right = 0.0f;
        if (keyboard_state[SDL_SCANCODE_S]) forward += 1.0f;
        if (keyboard_state[SDL_SCANCODE_W]) forward -= 1.0f;
        if (keyboard_state[SDL_SCANCODE_A]) right -= 1.0f;
        if (keyboard_state[SDL_SCANCODE_D]) right += 1.0f;
        
        move_camera(&camera, forward, right);

        // Handle mouse movement
        if (left_mouse_down) {
            move_camera_forward(&camera);
        }
        if (right_mouse_down) {
            move_camera_backward(&camera);
        }

        // Update chunks based on player position
        update_chunks(&world, camera.x, camera.z);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // Setup camera
        setup_camera(&camera);

        // Render world
        for (int x = 0; x < CHUNK_COUNT; x++) {
            for (int z = 0; z < CHUNK_COUNT; z++) {
                glPushMatrix();
                // Position chunks relative to the center chunk (player's chunk)
                float chunk_x = (x - VIEW_DISTANCE) * CHUNK_SIZE;
                float chunk_z = (z - VIEW_DISTANCE) * CHUNK_SIZE;
                glTranslatef(chunk_x, 0, chunk_z);
                render_chunk(&world.chunks[x][z]);
                glPopMatrix();
            }
        }

        // Swap buffers
        SDL_GL_SwapWindow(window);
        SDL_Delay(16); // Cap to ~60 FPS
    }

    // Cleanup
    cleanup_voxel_world(&world);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}