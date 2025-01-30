#version 450

#include "utils.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_tangent_bitangent_sign;
layout(location = 4) in vec3 in_color;
#if INSTANCING == 1
    layout(location = 5) in float shell_rank;
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


layout(binding = 0) uniform Data {
    FrameData frame;
};

uniform mat4 model;

// Fur
uniform float fur_rigidity;
uniform float fur_length;
uniform float hair_rigidity;
uniform float hair_fuzziness;
uniform float hair_fuzz_seed;
uniform float hair_curliness;
uniform float hair_curl_size; 

// Wind
uniform float wind_strength;
uniform float wind_alpha;
uniform float wind_beta;
uniform float turbulence_strength;
uniform float time;

// Movement
// uniform vec3 movememt;

// Rand function
float rand(vec2 seed) {
    uint n = uint(seed.x * 15731 + seed.y * 789221);
    n = (n << 13) ^ n;
    return 1.0 - float((n * (n * n * 15731u + 789221u) + 1376312589u & 0x7FFFFFFFu)) / 2147483648.0;
}

vec3 add_fuzz(vec3 vec)
{
    vec3 vec_norm = normalize(vec);
    vec3 up = cross(vec_norm, vec3(0., 1., 0.));
    vec3 left = cross(vec_norm, up);
    up = cross(vec_norm, left);

    float theta = hair_fuzz_seed + (rand(in_pos.xy) + rand(in_uv)) * 10.;

    float left_fuzz = sin(theta) + sin(3. * theta) * sin(5. * theta) - sin(5. * theta) * cos(0.5 * theta) - cos(3. * theta) / 2.;
    left_fuzz *= 0.5f;

    float up_fuzz = cos(theta) + sin(3. * theta) * sin(7. * theta) - sin(3. * theta) * cos(5. * theta) - cos(3. * theta) / 2.;
    up_fuzz *= 0.5f;

    vec3 fuzzed_vec = vec_norm + (up * up_fuzz + left * left_fuzz) * shell_rank * (hair_fuzziness / 10.);

    return normalize(fuzzed_vec);
}


vec3 add_curl(vec3 vec)
{
    vec3 vec_norm = normalize(vec);
    vec3 up = cross(vec_norm, vec3(0., 1., 0.));
    vec3 left = cross(vec_norm, up);
    up = cross(vec_norm, left);

    float theta = shell_rank * hair_curliness;

    float left_curl = sin(theta);
    float up_curl = cos(theta);

    float curl_size = .05;
    vec3 curled_vec = vec_norm + (up * up_curl + left * left_curl) * hair_curl_size;

    return normalize(curled_vec);
}

vec3 create_wind()
{
    float x = cos(wind_alpha);
    float y = sin(wind_alpha);
    float z = cos(wind_beta);

    vec3 wind = normalize(vec3(x, z, y)) * wind_strength;

    return wind;
}

vec3 add_turbulence(vec3 vec)
{
    vec3 vec_norm = normalize(vec);
    vec3 up = cross(vec_norm, vec3(0., 1., 0.));
    vec3 left = cross(vec_norm, up);
    up = cross(vec_norm, left);

    float theta = time + (rand(in_pos.xy) + rand(in_uv)) * 10.;

    float left_turb = sin(theta) + sin(3. * theta) * sin(5. * theta) - sin(5. * theta) * cos(0.5 * theta) - cos(3. * theta) / 2.;
    left_turb *= 0.5f;

    float up_turb = cos(theta) + sin(3. * theta) * sin(7. * theta) - sin(3. * theta) * cos(5. * theta) - cos(3. * theta) / 2.;
    up_turb *= 0.5f;

    vec3 turbed_vec = vec_norm + (up * up_turb + left * left_turb) * shell_rank * fur_length * 0.1 * turbulence_strength / (0.2 * hair_rigidity);

    return normalize(turbed_vec);
}

void main() {
    out_normal = normalize(mat3(model) * in_normal);
//    out_tangent = normalize(mat3(model) * in_tangent_bitangent_sign.xyz);
//    out_bitangent = cross(out_tangent, out_normal) * (in_tangent_bitangent_sign.w > 0.0 ? 1.0 : -1.0);

    /* --- Hair constraints --- */
    // Normal displacement of shell
    vec3 offset = out_normal;

    // Add fuzz
    offset = add_fuzz(offset);

    // Add curl
    offset = add_curl(offset);

    offset *= hair_rigidity;
    /* --- */

    /* --- Exterior constraints --- */
    // Gravity
    float dist = fur_length * shell_rank;
    vec3 gravity = 0.5f * vec3(0., -9.81, 0.) * dist;

    // Wind
    vec3 wind = create_wind();

    // Movement
    // vec3 movememt = (2. - shell_rank) * vec3(15. * sin(time), 0., 0.);

    // Compute displacement
    vec3 falloff = normalize(vec3(offset + gravity + wind));// + movememt));

    // Add turbulence
    falloff = add_turbulence(falloff) * dist;
    /* --- */

    // Compute position
    const vec4 position = model * vec4(in_pos, 1.0) + vec4(falloff, 0.);

    vec3 fall_in_normal = normalize(falloff);

    vec3 up = vec3(0.0, 1.0, 0.0);
    if (abs(dot(up, fall_in_normal)) > 0.99)
        up = vec3(1.0, 0.0, 0.0);
    out_tangent = normalize(mat3(model) * cross(up, fall_in_normal));
    out_bitangent = normalize(cross(out_tangent, out_normal));

    out_uv = in_uv;
    out_color = in_color;
    out_position = position.xyz;

    out_view_direction = normalize(frame.camera.position - position.xyz);
    #if INSTANCING == 1
    out_shell_rank = shell_rank;
    #endif

    gl_Position = frame.camera.view_proj * position;
}

