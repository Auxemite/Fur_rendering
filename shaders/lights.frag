#version 450

#include "utils.glsl"

// fragment shader of the main lighting pass

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_albedo_texture;
layout(binding = 1) uniform sampler2D in_normals_texture;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 1) buffer PointLights {
    PointLight point_lights[];
};

uniform uint render_mode;

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);
    const vec3 color = texelFetch(in_albedo_texture, coord, 0).rgb;
    const vec3 normal = texelFetch(in_normals_texture, coord, 0).rgb;

    vec3 acc = vec3(0.0);

    for(uint i = 0; i != frame.point_light_count; ++i) {
        PointLight light = point_lights[i];
        const vec3 to_light = (light.position);// - in_position);
        const float dist = length(to_light);
        const vec3 light_vec = to_light / dist;

        const float NoL = dot(light_vec, normal);
        const float att = attenuation(dist, light.radius);
        if(NoL <= 0.0 || att <= 0.0f) {
            continue;
        }

        acc += light.color * (NoL * att);
    }

    out_color = vec4(color * acc, 1.0);
}

