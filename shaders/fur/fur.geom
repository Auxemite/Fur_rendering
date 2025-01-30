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

vec3 get_tangent(vec3 pointa, vec3 pointb)
{
    vec3 a = normalize(pointa - pointb);
   return normalize(cross(normal, a));
}

vec3 col = vec3(.1f, .8f, 1.f);

void emit_vertex(vec3 point, int i)
{
    gl_Position = frame.camera.view_proj * model * vec4(point, 1.0);
    out_normal = mat3(model) * in_normal[i];
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

void emit_fin(int ia, int ib)
{
    float size = fur_length / float(shell_nb);
    emit_vertex(in_position[ia] - size * in_normal[ia], ia);
    emit_vertex(in_position[ib] - size * in_normal[ib], ib);
    emit_vertex(in_position[ia] + size * in_normal[ia], ia);
    emit_vertex(in_position[ib] + size * in_normal[ib], ib);

    EndPrimitive();
}

void emit_fins()
{
    for (int i = 0; i < 3; i++)
    {
        int next = (i + 1) % 3;
        vec3 tang = get_tangent(in_position[i], in_position[next]);

        if (abs(dot(view_direction, tang)) > .9 - fins_threshold)
            emit_fin(i, next);
    }
}

void emit_base_vertex(int i)
{
    gl_Position = gl_in[i].gl_Position;
    out_normal = in_normal[i];
    out_uv = in_uv[i];
    out_color = in_color[i];
    out_position = in_position[i];
    out_tangent = in_tangent[i];
    out_bitangent = in_bitangent[i];
    out_view_direction = in_view_direction[i];
    out_is_surface = 1;
    #if INSTANCING == 1
        out_shell_rank = shell_rank[i];
    #endif
    EmitVertex();
}


void emit_surface()
{
    for (int i = 0; i < 3; i++)
        emit_base_vertex(i);

    EndPrimitive();
}


void main()
{
    // The first few ranks never shows some fins
    // Then decide to show fin or normal surface depending
    // on angle between the camera and the surface
    float angle = abs(dot(view_direction, normal));

    if (show_shell == 1 && (shell_r < 0.2 || angle > fins_threshold))
        emit_surface();
    
    if (show_fins == 1 && angle <= fins_threshold)
        emit_fins();
}