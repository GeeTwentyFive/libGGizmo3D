#include "../libGGizmo3D.hpp"
#include <GWindower.hpp>
#include <glad/glad.h>
#include <GRenderer3D.hpp>
#include <GOBJ/GOBJ.hpp>
#include <linalg/linalg.h>
using namespace linalg::aliases;
#include <GFramePacer/GFramePacer.hpp>
#include <GLineRenderer3D.hpp>

#include <stdexcept>
#include <string>
#include <math.h>
#ifndef M_PI  // (Windows fix)
#define M_PI 3.14159265358979323846
#endif
#include <iostream>


const double TARGET_FRAMETIME_MS = 2.0;  // Should be a power-of-2 (like 1.0, 2.0, 4.0, 8.0) (to align as fraction/multiple of USB input report rate)

const int CAMERA_SENSITIVITY = 16;
const int CAMERA_MOVE_SPEED = 8;


#define ERROR(msg) throw std::runtime_error(std::string("[ERROR] ") + __FILE__ + "@" + std::to_string(__LINE__) + " (" + __func__ + "): " + (msg))


int main() { try {
        GWindower gw{0, 0, 4, 6};

        if (!gladLoadGL()) ERROR("Failed to load OpenGL");

        GRenderer3D gr3d; gr3d.camera_pos[2] = -2.0f;

        GOBJ cube_mesh_data("Cube.obj"); const uint32_t CUBE_TEXTURE[1] = {0xFFFFFFFF};
        uint64_t cube_mesh_id = gr3d.CreateMesh(
                (GRenderer3D::Vertex*)cube_mesh_data.vertices.data(), cube_mesh_data.vertices.size(),
                (uint32_t*)cube_mesh_data.indices.data(), cube_mesh_data.indices.size(),
                CUBE_TEXTURE, 1, 1
        );

        GRenderer3D::MeshInstance* cube_mesh_instance = gr3d.AddMesh(cube_mesh_id);

        GLineRenderer3D glr3d;

        glClearColor(0.0, 0.0, 0.0, 1.0);
        GFramePacer gfp; gfp.target_frametime_ms = TARGET_FRAMETIME_MS;
        for (;;) { // MAIN LOOP
                const double delta_time = gfp.Wait();

                const bool controlling_camera = gw.mouse_button_states[GWindower::MOUSEBUTTON_RIGHT];

                if (!gw.Update(controlling_camera) || gw.key_states[GWindower::KEY_ESCAPE]) break;

                if (controlling_camera) {
                        gr3d.camera_yaw += gw.mouse_x * CAMERA_SENSITIVITY;
                        gr3d.camera_pitch += gw.mouse_y * CAMERA_SENSITIVITY;
                        gr3d.camera_pos[1] += (-gw.key_states[GWindower::KEY_LEFT_CONTROL] + gw.key_states[GWindower::KEY_SPACE]) * CAMERA_MOVE_SPEED * delta_time;
                        float2 move_dir = float2{
                                (float)(-gw.key_states[GWindower::KEY_A] + gw.key_states[GWindower::KEY_D]),
                                (float)(-gw.key_states[GWindower::KEY_W] + gw.key_states[GWindower::KEY_S])
                        };
                        if (linalg::length2(move_dir) != 0) move_dir = linalg::normalize(move_dir);  // (to avoid division by zero)
                        move_dir = linalg::rot((float)(gr3d.camera_yaw*(2*M_PI/65536)), move_dir);
                        gr3d.camera_pos[0] += -move_dir.x * CAMERA_MOVE_SPEED * delta_time;
                        gr3d.camera_pos[2] += -move_dir.y * CAMERA_MOVE_SPEED * delta_time;

                        if (gw.key_states[GWindower::KEY_ZERO]) { gr3d.camera_pos[0] = 0; gr3d.camera_pos[1] = 0; gr3d.camera_pos[2] = 0; }
                }

                // -- GIZMO SECTION --

                // TODO

                glViewport(0, 0, gw.window_width, gw.window_height);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                gr3d.DrawFrame(gw.window_width, gw.window_height);
                glClear(GL_DEPTH_BUFFER_BIT);  // so that the gizmo draws over everything else
                glr3d.DrawFrame(gw.window_width, gw.window_height);
        }
return 0; } catch (const std::exception& e) { std::cout << e.what() << std::endl; return 1; } }