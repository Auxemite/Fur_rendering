#version 450

#define INSTANCING 1

layout(triangles)in;
layout(triangle_strip, max_vertices = 15)out;

layout(location = 0) in vec3 in_normal[];
layout(location = 1) in vec2 in_uv[];
layout(location = 2) in vec3 in_color[];
layout(location = 3) in vec3 in_position[];
layout(location = 4) in vec3 in_tangent[];
layout(location = 5) in vec3 in_bitangent[];
layout(location = 6) in vec3 in_view_direction[];
#if INSTANCING == 1
    layout(location = 7) in float shell_rank[];
    layout(location = 7) out float out_shell_rank;
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

float time()
{
    float speed = 0.3;
    return speed * anim_time;
}

vec3 get_normal_camera()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

vec3 get_normal()
{
   vec3 a = cPosition[0] - cPosition[1];
   vec3 b = cPosition[2] - cPosition[1];
   return normalize(cross(b, a));
}

vec3 normal = get_normal();

vec3 get_tangent(vec3 pointa, vec3 pointb)
{
    vec3 a = normalize(pointa - pointb);
   return normalize(cross(normal, a));
}

vec3 col = vec3(.1f, .8f, 1.f);

void emit_vertex(vec3 point)
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

void emit_fin(vec3 pointa, vec3 pointb)
{
    float dist = 0.02;
    emit_vertex(pointa);
    emit_vertex(pointa + dist * normal);
    emit_vertex(pointb);
    emit_vertex(pointb + dist * normal);

    EndPrimitive();
}

void emit_fins()
{
    for (int i = 0; i < SIZE; i++)
    {
        int next = (i + 1) % SIZE;
        vec3 tang = get_tangent(cPosition[i], cPosition[next]);

        if (abs(dot(vec3(0., 0., 1.), tang)) > 0.9)
            emit_fin(cPosition[i], cPosition[next]);
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
    #if INSTANCING == 1
        out_shell_rank = shell_rank[i];
    #endif
    EmitVertex();
}


void emit_surface()
{
    for (int i = 0; i < SIZE; i++)
        emit_base_vertex(i);

    EndPrimitive();
}


void main()
{
    emit_surface();
    // emit_fins();
}