#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_texture;

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);
    const vec3 color = texelFetch(in_texture, coord, 0).rgb;
    out_color = vec4(color, 1.0);
}


