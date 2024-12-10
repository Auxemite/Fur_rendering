#version 450

#include "utils.glsl"

// fragment shader of the main lighting pass

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_albedo_texture;
layout(binding = 1) uniform sampler2D in_normals_texture;
layout(binding = 2) uniform sampler2D in_depth_texture;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 1) buffer PointLights {
    PointLight point_lights[];
};

uniform uint render_mode;

vec3 unproject(vec2 uv, float depth, mat4 inv_viewproj) {
    const vec3 ndc = vec3(uv * 2.0 - vec2(1.0), depth);
    const vec4 p = inv_viewproj * vec4(ndc, 1.0);
    return p.xyz / p.w;
}

//const float width = 1600;
//const float height = 900;

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);
    const vec3 color = texelFetch(in_albedo_texture, coord, 0).rgb;
    const vec3 normal = texelFetch(in_normals_texture, coord, 0).rgb;
    const float depth = pow(texelFetch(in_depth_texture, coord, 0).r, 0.35);

    vec3 acc = vec3(0.0);
    mat4 inv_viewproj = inverse(frame.camera.view_proj);
//    const float coord_x = gl_FragCoord.x / width;
//    const float coord_y = gl_FragCoord.y / height;
    const vec3 in_position = unproject(in_uv, depth, inv_viewproj);

    for(uint i = 0; i != frame.point_light_count; ++i) {
        PointLight light = point_lights[i];
        const vec3 to_light = (light.position - in_position);
//        const vec3 to_light = light.position;
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

