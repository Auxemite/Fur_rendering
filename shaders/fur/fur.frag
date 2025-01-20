#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec3 in_position;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;

layout(binding = 0) uniform sampler2D in_texture;
layout(binding = 1) uniform sampler2D in_normal_texture;
uniform float density;
uniform float scale;

const vec3 ambient = vec3(0.0);

float rand(ivec2 seed) {
    uint n = uint(seed.x * 15731 + seed.y * 789221);
    n = (n << 13) ^ n;
    return 1.0 - float((n * (n * n * 15731u + 789221u) + 1376312589u & 0x7FFFFFFFu)) / 2147483648.0;
}

void main() {
//#ifdef NORMAL_MAPPED
//    const vec3 normal_map = unpack_normal_map(texture(in_normal_texture, in_uv).xy);
//    const vec3 normal = normal_map.x * in_tangent +
//                        normal_map.y * in_bitangent +
//                        normal_map.z * in_normal;
//#else
//    const vec3 normal = in_normal;
//#endif
    vec2 newUv = in_uv * 500.0;
    vec2 testUv = fract(newUv) * 2.0 - 1.0;
    ivec2 seed = ivec2(newUv);
    float random = rand(seed); // random value [0, 1]
    float dist = 10 / random; // lenght of the fur [0.1, 10]
    if (random < density) {
        vec3 color = vec3(in_uv, 1.0);
        if (length(testUv) < density / (random * scale)) {
            out_color = vec4(vec3(1.2 - density), 1.0) * texture(in_texture, in_uv);
//            out_color = texture(in_texture, in_uv);
        }
        else
            discard;
    }
    else
        discard;

//    out_color = vec4(in_color, 1.0);
//#ifdef TEXTURED
//    out_color *= texture(in_texture, in_uv);
//#endif
//    out_normal = vec4(normal, 1.0);
}

