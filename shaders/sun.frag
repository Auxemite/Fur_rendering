#version 450

#include "utils.glsl"

// fragment shader of the main lighting pass

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_albedo_texture;
layout(binding = 1) uniform sampler2D in_normals_texture;
layout(binding = 2) uniform sampler2D in_light_texture;

layout(binding = 0) uniform Data {
    FrameData frame;
};

uniform uint render_mode;

const vec3 ambient = vec3(0.0);

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);
    const vec3 color = texelFetch(in_albedo_texture, coord, 0).rgb;
    const vec3 normal = texelFetch(in_normals_texture, coord, 0).rgb;
    const vec3 light = texelFetch(in_light_texture, coord, 0).rgb;

    //    vec3 acc = vec3(1.0f) * max(0.0, dot(normalize(vec3(0.2f, 1.0f, 0.1f)), normal)) + ambient;
    vec3 acc = frame.sun_color * max(0.0, dot(frame.sun_dir, normal)) + ambient;

    out_color = vec4(light + color * acc, 1.0);
//    out_color = vec4(color * acc, 1.0);
}

