#version 450 core

#include "utils.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

layout(location = 0) in vec3 in_normal[];
layout(location = 1) in vec2 in_uv[];
layout(location = 2) in vec3 in_color[];
layout(location = 3) in vec3 in_position[];
layout(location = 4) in vec3 in_tangent[];
layout(location = 5) in vec3 in_bitangent[];
layout(location = 6) in vec3 in_view_direction[];
#if INSTANCING == 1
    layout(location = 7) in float shell_rank[];
#else
    uniform float shell_rank;
#endif
layout(location = 8) flat in int in_is_surface[];

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_color;
layout(location = 3) out vec3 out_position;
layout(location = 4) out vec3 out_tangent;
layout(location = 5) out vec3 out_bitangent;
layout(location = 6) out vec3 out_view_direction;
#if INSTANCING == 1
    layout(location = 7) out float out_shell_rank;
#endif
layout(location = 8) out int out_is_surface;

layout(binding = 0) uniform Data {
    FrameData frame;
};

// Uniforms
uniform mat4 model;
uniform float fur_length;
uniform uint shell_nb;
uniform float fins_threshold;
uniform uint show_shell;
uniform uint show_fins;

// Usefull variables
#if INSTANCING == 1
    float shell_r = shell_rank[0];
#else
    float shell_r = shell_rank;
#endif
vec3 normal = normalize(in_normal[0] + in_normal[1] + in_normal[2]);
vec3 view_direction = normalize(in_view_direction[0] + 
                                in_view_direction[1] + 
                                in_view_direction[2]);

void emit_vertex(vec3 point, int i)
{
    gl_Position = frame.camera.view_proj * vec4(point, 1.0);
    out_normal = in_normal[i];
    out_uv = in_uv[i];
    out_color = vec3(1., 1., 1.);//in_color[i];
    out_position = (model * vec4(point, 1.)).xyz;
    out_tangent = in_tangent[i];
    out_bitangent = in_bitangent[i];
    out_view_direction = in_view_direction[i];
    out_is_surface = 0;
    #if INSTANCING == 1
        out_shell_rank = shell_rank[i];
    #endif
    EmitVertex();
}

void emit_fin(int closest, int ia, int ib)
{
    float size = fur_length / float(shell_nb);
    
    float strength = 1. - abs(dot(view_direction, in_normal[closest]));
    // strength = 0;
    vec3 closest_extrude = in_position[closest] + size * in_normal[closest];
    vec3 ia_extrude = in_position[ia] + size * in_normal[ia];
    vec3 ib_extrude = in_position[ib] + size * in_normal[ib];
    vec3 c_ia = in_position[ia] - closest_extrude;
    vec3 c_ib = in_position[ib] - closest_extrude;

    emit_vertex(closest_extrude + strength * c_ia, closest);
    emit_vertex(closest_extrude + strength * c_ib, closest);
    emit_vertex(ib_extrude, ib);
    emit_vertex(ia_extrude, ia);

    EndPrimitive();
}

void adaptative_fins()
{
    
    int closest = 0;
    if (dot(view_direction, in_normal[closest]) < dot(view_direction, in_normal[1]))
        closest = 1;

    if (dot(view_direction, in_normal[closest]) < dot(view_direction, in_normal[2]))
        closest = 2;
    
    int ia = (closest + 1) % 3;
    int ib = (closest + 2) % 3;

    // emit_fin(closest, ia, ib);
    
    emit_fin(0, 1, 2);
    emit_fin(1, 2, 0);
    emit_fin(2, 0, 1);
}

void main()
{
    adaptative_fins();
}