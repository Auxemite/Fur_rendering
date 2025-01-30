#version 450 core

#include "utils.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

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

void main()
{
    for(int i = 0; i < 3; i++)
    {
        gl_Position = gl_in[i].gl_Position;
        out_normal = in_normal[i];
        out_uv = in_uv[i];
        out_color = in_color[i];
        out_position = in_position[i];
        out_tangent = in_tangent[i];
        out_bitangent = in_bitangent[i];
        out_view_direction = in_view_direction[i];
        #if INSTANCING == 1
            out_shell_rank = shell_rank[i];
        #endif
        EmitVertex();
    }
    EndPrimitive();
}