#version 450

#include "utils.glsl"

#define INSTANCING 0

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_tangent_bitangent_sign;
layout(location = 4) in vec3 in_color;
#if INSTANCING == 1
    layout(location = 5) in float shell_rank;
    layout(location = 6) out float out_shell_rank;
#else
    uniform float shell_rank;
#endif

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_color;
layout(location = 3) out vec3 out_position;
layout(location = 4) out vec3 out_tangent;
layout(location = 5) out vec3 out_bitangent;

layout(binding = 0) uniform Data {
    FrameData frame;
};

uniform mat4 model;

// Fur
uniform float density;
uniform float fur_length;
uniform float fur_rigidity;

// Wind
uniform float wind_strength;

void main() {
    out_normal = normalize(mat3(model) * in_normal);
    out_tangent = normalize(mat3(model) * in_tangent_bitangent_sign.xyz);
    out_bitangent = cross(out_tangent, out_normal) * (in_tangent_bitangent_sign.w > 0.0 ? 1.0 : -1.0);

    // Normal displacement of shell
    vec3 offset = out_normal * fur_rigidity;

    // Gravity
    float dist = fur_length * shell_rank;
    vec3 gravity = 0.5f * vec3(0., -9.81, 0.) * dist;

    // Wind
    vec3 wind = vec3(1., 0., 0.) * wind_strength;

    // Compute position
    vec3 falloff = normalize(offset + gravity + wind) * dist;
    const vec4 position = model * vec4(in_pos, 1.0) + vec4(falloff, 0.);

    out_uv = in_uv;
    out_color = in_color;
    out_position = position.xyz;

    #if INSTANCING == 1
        out_shell_rank = shell_rank;
    #endif

    gl_Position = frame.camera.view_proj * position;
}

