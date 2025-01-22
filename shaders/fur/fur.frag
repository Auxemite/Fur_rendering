#version 450

#include "utils.glsl"

#define INSTANCING 0

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec3 in_position;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;
#if INSTANCING == 1
    layout(location = 6) in float shell_rank;
#else
    uniform float shell_rank;
#endif

layout(binding = 0) uniform sampler2D in_texture;
layout(binding = 1) uniform sampler2D in_normal_texture;

uniform float density;
uniform float base_thickness;
uniform float tip_thickness;
uniform float min_length;
uniform float max_length;

const vec3 ambient = vec3(0.0);

float rand(vec2 seed) {
    uint n = uint(seed.x * 15731 + seed.y * 789221);
    n = (n << 13) ^ n;
    return 1.0 - float((n * (n * n * 15731u + 789221u) + 1376312589u & 0x7FFFFFFFu)) / 2147483648.0;
}

void main()
{
    vec2 uv = in_uv * density;
    vec2 uv_fract = fract(uv) * 2.0 - 1.0;
    ivec2 seed = ivec2(uv);
    float random = rand(ivec2(uv)); // random value [0, 1]
    float thickness =  base_thickness + (shell_rank / random) * (tip_thickness - base_thickness);

    if (random > shell_rank && random >= min_length && random <= max_length)
    {
        vec3 color = vec3(in_uv, 1.0);
        
        if (length(uv_fract) <= thickness)
        {
            out_color = vec4(vec3(0.5 + 0.5 * shell_rank), 1.0) * texture(in_texture, in_uv);
            // out_color = texture(in_texture, in_uv);
        }
        else
            discard;
    }
    else
        discard;
}

