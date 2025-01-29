
#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <graphics.h>
#include <Scene.h>
#include <Texture.h>
#include <Framebuffer.h>
#include <TimestampQuery.h>
#include <ImGuiRenderer.h>

#include <imgui/imgui.h>

#include <iostream>
#include <vector>
#include <filesystem>

using namespace OM3D;

static RenderMode render_mode = RenderMode::Default;
static float delta_time = 0.0f;
static float total_time = 0.0f;
static std::unique_ptr<Scene> scene;
static float exposure = 1.0;
static std::vector<std::string> scene_files;

namespace OM3D {
extern bool audit_bindings_before_draw;
}

void parse_args(int argc, char** argv) {
    for(int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];

        if(arg == "--validate") {
            OM3D::audit_bindings_before_draw = true;
        } else {
            std::cerr << "Unknown argument \"" << arg << "\"" << std::endl;
        }
    }
}

void glfw_check(bool cond) {
    if(!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void update_delta_time() {
    static double time = 0.0;
    const double new_time = program_time();
    delta_time = float(new_time - time);
    total_time += delta_time;
    time = new_time;
}

void process_inputs(GLFWwindow* window, Camera& camera) {
    static glm::dvec2 mouse_pos;

    glm::dvec2 new_mouse_pos;
    glfwGetCursorPos(window, &new_mouse_pos.x, &new_mouse_pos.y);

    {
        glm::vec3 movement = {};
        if(glfwGetKey(window, 'W') == GLFW_PRESS) {
            movement += camera.forward();
        }
        if(glfwGetKey(window, 'S') == GLFW_PRESS) {
            movement -= camera.forward();
        }
        if(glfwGetKey(window, 'D') == GLFW_PRESS) {
            movement += camera.right();
        }
        if(glfwGetKey(window, 'A') == GLFW_PRESS) {
            movement -= camera.right();
        }

        float speed = 10.0f;
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            speed *= 10.0f;
        }

        if(movement.length() > 0.0f) {
            const glm::vec3 new_pos = camera.position() + movement * delta_time * speed;
            camera.set_view(glm::lookAt(new_pos, new_pos + camera.forward(), camera.up()));
        }
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        const glm::vec2 delta = glm::vec2(mouse_pos - new_mouse_pos) * 0.01f;
        if(delta.length() > 0.0f) {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta.x, glm::vec3(0.0f, 1.0f, 0.0f));
            rot = glm::rotate(rot, delta.y, camera.right());
            camera.set_view(glm::lookAt(camera.position(), camera.position() + (glm::mat3(rot) * camera.forward()), (glm::mat3(rot) * camera.up())));
        }
    }

    {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(window, &width, &height);
        camera.set_ratio(float(width) / float(height));
    }

    mouse_pos = new_mouse_pos;
}

void gui(ImGuiRenderer& imgui) {
    const ImVec4 error_text_color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
    const ImVec4 warning_text_color = ImVec4(1.0f, 0.8f, 0.4f, 1.0f);

    static bool open_gpu_profiler = false;

    PROFILE_GPU("GUI");

    imgui.start();
    DEFER(imgui.finish());

    // ImGui::ShowDemoWindow();

    bool open_scene_popup = false;
    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            if(ImGui::MenuItem("Open Scene")) {
                open_scene_popup = true;
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Exposure")) {
            ImGui::DragFloat("Exposure", &exposure, 0.25f, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
            if(exposure != 1.0f && ImGui::Button("Reset")) {
                exposure = 1.0f;
            }
            ImGui::EndMenu();
        }

        if(scene && ImGui::BeginMenu("Scene Info")) {
            ImGui::Text("%u objects", u32(scene->objects().size()));
            ImGui::Text("%u point lights", u32(scene->point_lights().size()));
            ImGui::EndMenu();
        }

        ImGui::Separator();
        if(ImGui::BeginMenu("Debug")) {
            if(ImGui::MenuItem("None")) {
                render_mode = RenderMode::Default;
            }
            if(ImGui::MenuItem("Albedo")) {
                render_mode = RenderMode::Albedo;
            }
            if(ImGui::MenuItem("Normals")) {
                render_mode = RenderMode::Normals;
            }
            if(ImGui::MenuItem("Depth")) {
                render_mode = RenderMode::Depth;
            }
            if(ImGui::MenuItem("Tangent")) {
                render_mode = RenderMode::Tangent;
            }
            if(ImGui::MenuItem("Bitangent")) {
                render_mode = RenderMode::Bitangent;
            }
            ImGui::EndMenu();
        }
        ImGui::Separator();

        if(ImGui::BeginMenu("Fur options")) {
            ImGui::DragInt("Shell number", &shell_number, 1.f, 1, 200); 
            ImGui::DragFloat("Fur Density", &fur_density, 1.f, 1.0f, 800.0f, "%.1f");
            ImGui::DragFloat("Hair Rigidity", &hair_rigidity, 1.f, 0.1f, 100.0f, "%.1f");
            ImGui::DragFloat("Hair Length", &fur_length, .1f, 0.5f, 50.f, "%.2f", ImGuiSliderFlags_None);
            ImGui::DragFloat("Base thickness", &base_thickness, .01f, 0.0f, 2.0f, "%.3f");
            ImGui::DragFloat("Tip thickness", &tip_thickness, .01f, 0.0f, 2.0f, "%.3f");
            ImGui::DragFloat("Min Length", &hair_min_length, .01f, 0.0f, 1.0f, "%.3f");
            ImGui::DragFloat("Max Length", &hair_max_length, .01f, 0.0f, 1.0f, "%.3f");
            ImGui::DragFloat("Hair Fuzziness", &hair_fuzziness, .01f, 0.0f, 10.f, "%.2f");
            ImGui::DragFloat("Hair Fuzz_seed", &hair_fuzz_seed, .01f, 0.0f, 10.f, "%.2f");
            ImGui::DragFloat("Hair Curliness", &hair_curliness, .01f, 0.0f, 50.f, "%.2f");
            ImGui::DragFloat("Hair Curl_size", &hair_curl_size, .01f, 0.0f, 1.0f, "%.3f");
          
            if(ImGui::Button("Reset")) {
                shell_number = 32;
                fur_density = 325.f;
                hair_rigidity = 25.f;
                base_thickness = 1.5f; // [0. - 1.5]
                tip_thickness = .05f; // [0. - 1.5]
                fur_length = 1.f;
                hair_min_length = 0.f; // [0. - 1.]
                hair_max_length = 1.f; // [0. - 1.]
                hair_fuzziness = 2.f; // [0. - 10.]
                hair_fuzz_seed = 2.f; // [0. - 10.]
                hair_curliness = 2.f; // [0. - 50.]
                hair_curl_size = .2f; // [0. - 1.]
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("BRDF options")) {
            ImGui::Checkbox("Kajiya-Kay", &kajyia_Kay);
            ImGui::DragFloat("Fur Lighting", &fur_lighting, 0.1f, 0.0f, 5.0f, "%.1f");
            ImGui::DragFloat("Roughness", &roughness, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("Metaless", &metaless, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("Ambient", &ambient, 0.01f, 0.0f, 1.0f, "%.2f");
            ImGui::DragFloat("Ambient Occlusion", &ambient_occlusion, 0.01f, 0.0f, 1.0f, "%.2f");
            if(ImGui::Button("Reset")) {
                fur_lighting = 0.0f;
                roughness = 0.6f;
                metaless = 0.0f;
                ambient = 0.0f;
                ambient_occlusion = 1.0f;
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Wind options")) {
            ImGui::DragFloat("Wind Strength", &wind_strength, .1f, -50.0f, 50.0f, "%.1f");
            ImGui::DragFloat("Turbulence Strength", &turbulence_strength, .01f, 0.f, 10.0f, "%.2f");
            ImGui::DragFloat("Wind Direction (xy)", &wind_alpha, .01f, -10.f, 10.f, "%.2f");
            ImGui::DragFloat("Wind Direction (z)", &wind_beta, .01f, 0.f, 10.f, "%.2f");
            if(ImGui::Button("Reset")) {
                wind_strength = 10.f;
                wind_alpha = 0.f; // [-10. - 10.]
                wind_beta = 5.f; // // [0. - 10.]
                turbulence_strength = 2.f; // // [0. - 10.]
            }
            ImGui::EndMenu();
        }

        if(ImGui::MenuItem("GPU Profiler")) {
            open_gpu_profiler = true;
        }

        ImGui::Separator();
        ImGui::TextUnformatted(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

        ImGui::Separator();
        ImGui::Text("%.2f ms", delta_time * 1000.0f);

#ifdef OM3D_DEBUG
        ImGui::Separator();
        ImGui::TextColored(warning_text_color, ICON_FA_BUG " (DEBUG)");
#endif

        if(!bindless_enabled()) {
            ImGui::Separator();
            ImGui::TextColored(error_text_color, ICON_FA_EXCLAMATION_TRIANGLE " Bindless textures not supported");
        }
        ImGui::EndMainMenuBar();
    }

    if(open_scene_popup) {
        ImGui::OpenPopup("###openscenepopup");

        scene_files.clear();
        for(auto&& entry : std::filesystem::directory_iterator(data_path)) {
            if(entry.status().type() == std::filesystem::file_type::regular) {
                const auto ext = entry.path().extension();
                if(ext == ".gltf" || ext == ".glb") {
                    scene_files.emplace_back(entry.path().string());
                }
            }
        }
    }

    if(ImGui::BeginPopup("###openscenepopup", ImGuiWindowFlags_AlwaysAutoResize)) {
        auto load_scene = [](const std::string path) {
            auto result = Scene::from_gltf(path);
            if(!result.is_ok) {
                std::cerr << "Unable to load scene (" << path << ")" << std::endl;
            } else {
                scene = std::move(result.value);
            }
            ImGui::CloseCurrentPopup();
        };

        char buffer[1024] = {};
        if(ImGui::InputText("Load scene", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            load_scene(buffer);
        }

        if(!scene_files.empty()) {
            for(const std::string& p : scene_files) {
                const auto abs = std::filesystem::absolute(p).string();
                if(ImGui::MenuItem(abs.c_str())) {
                    load_scene(p);
                    break;
                }
            }
        }

        ImGui::EndPopup();
    }

    if(open_gpu_profiler) {
        if(ImGui::Begin(ICON_FA_CLOCK " GPU Profiler")) {
            const ImGuiTableFlags table_flags =
                ImGuiTableFlags_SortTristate |
                ImGuiTableFlags_NoSavedSettings |
                ImGuiTableFlags_SizingFixedFit |
                ImGuiTableFlags_BordersInnerV |
                ImGuiTableFlags_Resizable |
                ImGuiTableFlags_RowBg;

            ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4(1, 1, 1, 0.01f));
            DEFER(ImGui::PopStyleColor());

            if(ImGui::BeginTable("##timetable", 3, table_flags)) {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("CPU (ms)", ImGuiTableColumnFlags_NoResize, 70.0f);
                ImGui::TableSetupColumn("GPU (ms)", ImGuiTableColumnFlags_NoResize, 70.0f);
                ImGui::TableHeadersRow();

                std::vector<u32> indents;
                for(const auto& zone : retrieve_profile()) {
                    auto color_from_time = [](float time) {
                        const float t = std::min(time / 0.008f, 1.0f); // 8ms = red
                        return ImVec4(t, 1.0f - t, 0.0f, 1.0f);
                    };

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(zone.name.data());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushStyleColor(ImGuiCol_Text, color_from_time(zone.cpu_time));
                    ImGui::Text("%.2f", zone.cpu_time * 1000.0f);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushStyleColor(ImGuiCol_Text, color_from_time(zone.gpu_time));
                    ImGui::Text("%.2f", zone.gpu_time * 1000.0f);

                    ImGui::PopStyleColor(2);

                    if(!indents.empty() && --indents.back() == 0) {
                        indents.pop_back();
                        ImGui::Unindent();
                    }

                    if(zone.contained_zones) {
                        indents.push_back(zone.contained_zones);
                        ImGui::Indent();
                    }
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}




std::unique_ptr<Scene> create_default_scene(const std::string& scene_file) {
    auto new_scene = std::make_unique<Scene>();
    // Load default cube model

    auto result = Scene::from_gltf(std::string(data_path) + scene_file);
    ALWAYS_ASSERT(result.is_ok, "Unable to load default scene");
    new_scene = std::move(result.value);

    new_scene->set_sun(glm::vec3(0.2f, 1.0f, 0.1f), glm::vec3(1.0f));

    // Add lights
//    {
//        PointLight light;
//        light.set_position(glm::vec3(1.0f, 2.0f, 4.0f));
//        light.set_color(glm::vec3(0.0f, 50.0f, 0.0f));
//        light.set_radius(100.0f);
//        scene->add_light(light);
//    }
//    {
//        PointLight light;
//        light.set_position(glm::vec3(1.0f, 2.0f, -4.0f));
//        light.set_color(glm::vec3(50.0f, 0.0f, 0.0f));
//        light.set_radius(50.0f);
//        scene->add_light(light);
//    }

    return new_scene;
}

struct RendererState {
    static RendererState create(glm::uvec2 size) {
        RendererState state;

        state.size = size;

        if(state.size.x > 0 && state.size.y > 0) {
            state.depth_texture = Texture(size, ImageFormat::Depth32_FLOAT);
            state.albedo_texture = Texture(size, ImageFormat::RGBA8_sRGB);
            state.normal_texture = Texture(size, ImageFormat::RGBA8_UNORM);
            state.tangent_texture = Texture(size, ImageFormat::RGBA8_UNORM);
            state.bitangent_texture = Texture(size, ImageFormat::RGBA8_UNORM);
            state.lit_hdr_texture = Texture(size, ImageFormat::RGBA16_FLOAT);
            state.tone_mapped_texture = Texture(size, ImageFormat::RGBA8_UNORM);

            state.depth_framebuffer = Framebuffer(&state.depth_texture);
            state.main_framebuffer = Framebuffer(&state.depth_texture, std::array{&state.lit_hdr_texture});
//            state.g_buffer_framebuffer = Framebuffer(&state.depth_texture, std::array{&state.albedo_texture, &state.normal_texture});
            state.g_buffer_framebuffer = Framebuffer(&state.depth_texture, std::array{&state.albedo_texture, &state.normal_texture, &state.tangent_texture, &state.bitangent_texture});
            state.lit_framebuffer = Framebuffer(nullptr, std::array{&state.lit_hdr_texture});
            state.tone_map_framebuffer = Framebuffer(nullptr, std::array{&state.tone_mapped_texture});
        }
        return state;
    }

    glm::uvec2 size = {};

    Texture depth_texture;
    Texture lit_hdr_texture;
    Texture albedo_texture;
    Texture normal_texture;
    Texture tangent_texture;
    Texture bitangent_texture;
    Texture tone_mapped_texture;

    Framebuffer depth_framebuffer;
    Framebuffer test_framebuffer;
    Framebuffer tone_map_framebuffer;
    Framebuffer g_buffer_framebuffer;
    Framebuffer main_framebuffer;
    Framebuffer lit_framebuffer;
};

int main(int argc, char** argv) {
    DEBUG_ASSERT([] { std::cout << "Debug asserts enabled" << std::endl; return true; }());

    parse_args(argc, argv);

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(1600, 900, "OM3D", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    init_graphics();

    ImGuiRenderer imgui(window);

//    scene = create_default_scene("bistro_lights.glb");
//    scene = create_default_scene("forest.glb");
    scene = create_default_scene("cube.glb");
//    scene = create_default_scene("forest_huge.glb");

    const std::unique_ptr<Scene> sphere_scene = create_default_scene("sphere2.glb");
    SceneObject sphere = sphere_scene->objects()[0];
    Material material = Material::textured_normal_mapped_material();
    sphere.set_material(std::make_shared<Material>(material));
    scene->add_object(sphere);
    scene->delete_object(0);
//    scene = create_default_scene("rock.glb");
    
    std::vector<PointLight> lights;
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, 4.0f));
        light.set_color(glm::vec3(0.0f, 50.0f, 0.0f));
        light.set_radius(100.0f);
        lights.push_back(light);
    }
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, -4.0f));
        light.set_color(glm::vec3(50.0f, 0.0f, 0.0f));
        light.set_radius(50.0f);
        lights.push_back(light);
    }

//    for (const auto & light : lights) {
//        const glm::vec3& pos = light.position();
//        sphere_scene->copy_object(0, pos);
//    }
    // print infos of the scene objects
//    for(const SceneObject& obj : sphere_scene->objects())
//    {
//        obj.print_info();
//    }

    auto tonemap_program = Program::from_files("tonemap.frag", "screen.vert", "");
    auto debug_program = Program::from_files("debug.frag", "screen.vert", "");
    auto sun_program = Program::from_files("sun.frag", "screen.vert", "");
    auto lights_program = Program::from_files("lights.frag", "screen.vert", "");
    RendererState renderer;

    int i = 0;
    for(;;) {
        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        process_profile_markers();

        {
            int width = 0;
            int height = 0;
            glfwGetWindowSize(window, &width, &height);

            if(renderer.size != glm::uvec2(width, height)) {
                renderer = RendererState::create(glm::uvec2(width, height));
            }
        }

        update_delta_time();

        if(const auto& io = ImGui::GetIO(); !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            process_inputs(window, scene->camera());
        }

        // Draw everything
        {
            PROFILE_GPU("Frame");
            // Render for Z prepass
            {
                PROFILE_GPU("Zprepass");

                renderer.depth_framebuffer.bind(true, false);
                scene->render(RenderMode::GBuffer, i, false, total_time);
            }

            // Render the scene
            {
                PROFILE_GPU("G Buffer pass");

                renderer.g_buffer_framebuffer.bind(false, true);
                scene->render(RenderMode::GBuffer, i, false, total_time);
            }

            {
                PROFILE_GPU("Main pass");

                renderer.main_framebuffer.bind(false, true);
                scene->render(render_mode, ++i, true, total_time);
                i %= 60;
            }

//            {
//                PROFILE_GPU("Lighting pass : Point lights");
//
//                TypedBuffer<shader::FrameData> buffer = scene->get_sun_frame_data();
//                TypedBuffer<shader::PointLight> light_buffer = scene->get_lights_frame_data();
//                // print light radiuses
//
//                glFrontFace(GL_CW);
//                renderer.lit_framebuffer.bind(false, true);
//                buffer.bind(BufferUsage::Uniform, 0);
//                light_buffer.bind(BufferUsage::Storage, 1);
//                renderer.albedo_texture.bind(0);
//                renderer.normal_texture.bind(1);
//                renderer.depth_texture.bind(2);
//                lights_program->bind();
//                glDrawArrays(GL_TRIANGLES, 0, 3);
//            }
//            {
//                PROFILE_GPU("Lighting pass : Sun");
//
//                TypedBuffer<shader::FrameData> buffer = scene->get_sun_frame_data();
//                glFrontFace(GL_CW);
//                renderer.lit_framebuffer.bind(false, false);
//                buffer.bind(BufferUsage::Uniform, 0);
//                renderer.albedo_texture.bind(0);
//                renderer.normal_texture.bind(1);
//                renderer.lit_hdr_texture.bind(2);
//                sun_program->bind();
//                glDrawArrays(GL_TRIANGLES, 0, 3);
//            }

            // Apply a tonemap in compute shader
            {
                PROFILE_GPU("Tonemap");

                // Tone Mapping Triangle is Facing away from camera
                glFrontFace(GL_CW);

                renderer.tone_map_framebuffer.bind(false, false);
                if (render_mode != RenderMode::Default) {
                    debug_program->bind();
                    debug_program->set_uniform(HASH("render_mode"), static_cast<u32>(render_mode));
                    switch (render_mode)
                    {
                        case RenderMode::Normals:
                            renderer.normal_texture.bind(0);
                            break;

                        case RenderMode::Depth:
                            renderer.depth_texture.bind(0);
                            break;

                        case RenderMode::Tangent:
                            renderer.tangent_texture.bind(0);
                            break;

                        case RenderMode::Bitangent:
                            renderer.bitangent_texture.bind(0);
                            break;

                        default:
                            renderer.albedo_texture.bind(0);
                            break;
                    }

                    glDrawArrays(GL_TRIANGLES, 0, 3);
                }
                else {
                    tonemap_program->bind();
                    tonemap_program->set_uniform(HASH("exposure"), exposure);
                    renderer.lit_hdr_texture.bind(0);
                }
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }

            // Blit tonemap result to screen
            {
                PROFILE_GPU("Blit");

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                renderer.tone_map_framebuffer.blit();
            }

            // Draw GUI on top
            gui(imgui);
        }

        glfwSwapBuffers(window);
    }

    scene = nullptr; // destroy scene and child OpenGL objects
    return 0;
}
