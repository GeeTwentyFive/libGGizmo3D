#include "../libGGizmo3D.hpp"
#include <GWindower.hpp>
#include <glad/glad.h>
#include <GRenderer3D.hpp>
#include <GOBJ/GOBJ.hpp>
#include <linalg/linalg.h>
using namespace linalg::aliases;
#include <GFramePacer/GFramePacer.hpp>
#include <GLineRenderer3D.hpp>
#include <libGMisc3DMath/libGMisc3DMath.hpp>

#include <stdexcept>
#include <string>
#include <math.h>
#ifndef M_PI  // (Windows fix)
#define M_PI 3.14159265358979323846
#endif
#include <iostream>


const float GIZMO_WIDGET_LENGTH = 1.0f/4.0f;
const float GIZMO_WIDGET_WIDTH = 1.0f/32.0f;
const uint32_t GIZMO_WIDGET_COLORS[GGizmo3D::WidgetIndex::_count] = {
        0xFF0000FF, 0x800000FF, // X+, X-
        0x00FF00FF, 0x008000FF, // Y+, Y-
        0x0000FFFF, 0x000080FF  // Z+, Z-
};

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

                static int previous_mouse_x = gw.window_width, previous_mouse_y = gw.window_height;
                const int mouse_x_delta = gw.mouse_x - previous_mouse_x;
                const int mouse_y_delta = gw.mouse_y - previous_mouse_y;

                // -- GIZMO SECTION --

                static bool select_gizmo = false, last_select_gizmo_state = false;
                last_select_gizmo_state = select_gizmo;
                select_gizmo = gw.mouse_button_states[GWindower::MOUSEBUTTON_LEFT];
                const bool select_gizmo_just_pressed = select_gizmo && (last_select_gizmo_state == false);

                static GGizmo3D::WidgetIndex active_gizmo_widget = GGizmo3D::WidgetIndex::NONE;

                const float gizmo_scale = linalg::length(*(float3*)cube_mesh_instance->position - *(float3*)gr3d.camera_pos);
                GGizmo3D gizmo(
                        *(float3*)cube_mesh_instance->position,
                        GIZMO_WIDGET_LENGTH * gizmo_scale, GIZMO_WIDGET_WIDTH * gizmo_scale
                );

                if (!select_gizmo || controlling_camera) active_gizmo_widget = GGizmo3D::WidgetIndex::NONE;
                else if (select_gizmo_just_pressed) {
                        active_gizmo_widget = gizmo.Pick(
                                *(float3*)gr3d.camera_pos,
                                GMisc3DMath::ScreenToWorldRay(gw.mouse_x, gw.mouse_y, gw.window_width, gw.window_height, (float4x4*)gr3d.ViewProjection())
                        );
                }
                switch (active_gizmo_widget) { default: { for (int i = 0; i < GGizmo3D::WidgetIndex::_count; i++) { glr3d.DrawBox(gizmo.aabbs[i].min.x, gizmo.aabbs[i].min.y, gizmo.aabbs[i].min.z, gizmo.aabbs[i].max.x, gizmo.aabbs[i].max.y, gizmo.aabbs[i].max.z, GIZMO_WIDGET_COLORS[i]); } } break;
                        case GGizmo3D::WidgetIndex::X_POSITIVE: {
                                // TODO
                        } break;
                        case GGizmo3D::WidgetIndex::X_NEGATIVE: {
                                // TODO
                        } break;
                        case GGizmo3D::WidgetIndex::Y_POSITIVE: {
                                // TODO
                        } break;
                        case GGizmo3D::WidgetIndex::Y_NEGATIVE: {
                                // TODO
                        } break;
                        case GGizmo3D::WidgetIndex::Z_POSITIVE: {
                                // TODO
                        } break;
                        case GGizmo3D::WidgetIndex::Z_NEGATIVE: {
                                // TODO
                        } break;
                }

                previous_mouse_x = gw.mouse_x; previous_mouse_y = gw.mouse_y;

                glViewport(0, 0, gw.window_width, gw.window_height);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                gr3d.DrawFrame(gw.window_width, gw.window_height);
                glClear(GL_DEPTH_BUFFER_BIT);  // so that the gizmo draws over everything else
                glr3d.camera_pos[0] = gr3d.camera_pos[0]; glr3d.camera_pos[1] = gr3d.camera_pos[1]; glr3d.camera_pos[2] = gr3d.camera_pos[2]; glr3d.camera_yaw = gr3d.camera_yaw; glr3d.camera_pitch = gr3d.camera_pitch; glr3d.camera_fov = gr3d.camera_fov; glr3d.camera_near = gr3d.camera_near; glr3d.camera_far = gr3d.camera_far;
                glr3d.DrawFrame(gw.window_width, gw.window_height);
        }
return 0; } catch (const std::exception& e) { std::cout << e.what() << std::endl; return 1; } }