#version 450

#include "utils.glsl"

#define INSTANCING 1

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_tangent_bitangent_sign;
layout(location = 4) in vec3 in_color;
#if INSTANCING == 1
    layout(location = 5) in float shell_rank;
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
uniform float wind_alpha;
uniform float wind_beta;
uniform float time;

float rand(vec2 seed) {
    uint n = uint(seed.x * 15731 + seed.y * 789221);
    n = (n << 13) ^ n;
    return 1.0 - float((n * (n * n * 15731u + 789221u) + 1376312589u & 0x7FFFFFFFu)) / 2147483648.0;
}

vec3 create_wind()
{
    float x = cos(wind_alpha);
    float y = sin(wind_alpha);
    float z = cos(wind_beta);

    vec3 wind = normalize(vec3(x, z, y)) * wind_strength;

    return wind;
}

vec3 create_turbulence(vec3 vec)
{
    vec3 vec_norm = normalize(vec);
    vec3 up = cross(vec_norm, vec3(0., 1., 0.));
    vec3 left = cross(vec_norm, up);
    up = cross(vec_norm, left);

    float theta = time + rand(in_pos.xy) + rand(in_uv);

    float left_turb = sin(theta) + sin(3. * theta) * sin(5. * theta) - sin(7. * theta) * cos(2. * theta) / 14. - cos(4. * theta) / 4.;
    left_turb *= .5f;

    float up_turb = - left_turb;

    return (up * 0. + left * left_turb) * pow(0.1 * wind_strength, 2.);
}

void main() {
    out_normal = normalize(mat3(model) * in_normal);
////    out_tangent = normalize(mat3(model) * in_tangent_bitangent_sign.xyz);
////    out_bitangent = cross(out_tangent, out_normal) * (in_tangent_bitangent_sign.w > 0.0 ? 1.0 : -1.0);
//    out_tangent = normalize(mat3(model) * cross(up, in_normal));
//    out_bitangent = normalize(cross(out_tangent, out_normal));

    // Normal displacement of shell
    vec3 offset = out_normal * fur_rigidity;

    // Gravity
    float dist = fur_length * shell_rank;
    vec3 gravity = 0.5f * vec3(0., -9.81, 0.) * dist;

    // Wind
    vec3 wind = create_wind();
    wind += create_turbulence(offset + gravity + wind);

    // Compute position
    vec3 falloff = normalize(vec3(offset + gravity + wind)) * dist;

    const vec4 position = model * vec4(in_pos, 1.0) + vec4(falloff, 0.);

    out_uv = in_uv;
    out_color = in_color;
    out_color.x = abs(dot(out_normal, falloff));
    out_position = position.xyz;

    out_view_direction = normalize(frame.camera.position - position.xyz);
    #if INSTANCING == 1
        out_shell_rank = shell_rank;
    #endif
    
    gl_Position = frame.camera.view_proj * position;
}

