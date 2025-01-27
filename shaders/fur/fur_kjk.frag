#version 450

#include "utils.glsl"

#define INSTANCING 1

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec3 in_position;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;
layout(location = 6) in vec3 out_view_direction;
#if INSTANCING == 1
layout(location = 7) in float shell_rank;
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

uniform float fur_lighting;
uniform float roughness;
uniform float metaless;
uniform float ambient;
uniform float ambient_occlution;

float pi = 3.14159265359;

vec4 sRGBToLinear( in vec4 value ) {
    return vec4( mix( pow( value.rgb * 0.9478672986 + vec3( 0.0521327014 ), vec3( 2.4 ) ), value.rgb * 0.0773993808, vec3( lessThanEqual( value.rgb, vec3( 0.04045 ) ) ) ), value.a );
}

vec3 Aces(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec4 LinearTosRGB( in vec4 value ) {
    return vec4( mix( pow( value.rgb, vec3( 0.41666 ) ) * 1.055 - vec3( 0.055 ), value.rgb * 12.92, vec3( lessThanEqual( value.rgb, vec3( 0.0031308 ) ) ) ), value.a );
}

float rand(vec2 seed) {
    uint n = uint(seed.x * 15731 + seed.y * 789221);
    n = (n << 13) ^ n;
    return 1.0 - float((n * (n * n * 15731u + 789221u) + 1376312589u & 0x7FFFFFFFu)) / 2147483648.0;
}

float rdot(vec3 a, vec3 b) {
    return max(0.0, dot(a, b));
}

float fresnelSchlick(float dotVH, float f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - dotVH, 5.0);
}

// Kajiya-Kay shading model function
vec3 kajiya_kay(vec3 tangent, vec3 bitangent, vec3 normal, vec3 light_dir,
            vec3 view_dir, vec3 albedo, vec3 light_color, float random)
{
    // Compute the effective normal
    vec3 strand_normal = normalize(light_dir - tangent * dot(tangent, light_dir));
    vec3 half_vector = normalize(light_dir + view_dir);

    // Diffuse term
//    vec3 h = normalize(lightVec + viewVec);
//    float ks = fresnelSchlick(rdot(h, viewVec), 0.04 + metaless);
//    vec3 diffuse = (1.0 - ks) * (texture(in_texture, in_uv)).xyz * vec3(0.1, 0.0, 0.0) * rdot(normal, lightVec) / pi;
    float diffuse = max(dot(strand_normal, light_dir), 0.0);
//    diffuse = 0.0;

    // Specular term
    float specular = metaless * pow(max(dot(strand_normal, half_vector), 0.0), 10.0 / (roughness + 0.001));
    specular += 0.1 * pow(max(dot(strand_normal, half_vector), 0.0), 1.0 / (roughness + 0.001));
//    specular = 0.0;

    // Combine lighting
    vec3 color = (diffuse * albedo * random + specular * light_color) * light_color;
    return color;
}

void main()
{
    vec2 uv = in_uv * density;
    vec2 uv_fract = fract(uv) * 2.0 - 1.0;
    ivec2 seed = ivec2(uv.x, uv.y);
    ivec2 seed2 = ivec2(uv.y, uv.x);
    float random = rand(seed); // random value [0, 1]
    float random2 = 1.0; //rand(seed2) * 0.5 + 0.5; // random value [0, 1]
    float thickness =  base_thickness + (shell_rank / random) * (tip_thickness - base_thickness);
    float fur_deepness = pow(0.5 + 0.5 * shell_rank / thickness, fur_lighting);

    if (shell_rank == 0) {
        vec3 albedo = vec3(1.0) * texture(in_texture, in_uv).rgb;
        vec3 final_color = fur_deepness * (albedo + vec3(ambient));
        final_color = sRGBToLinear(vec4(final_color, 1.0)).rgb;
        final_color = Aces(final_color); // HDR tone mapping
        out_color = LinearTosRGB(vec4(final_color, 1.0));
    }
    else if (random > shell_rank && random >= min_length && random <= max_length)
    {
        if (length(uv_fract) <= thickness)
        {
            // Sample textures
            vec3 albedo = vec3(0.1, 0.0, 0.0) * texture(in_texture, in_uv).rgb;

            vec3 light_direction = vec3(5.0, 5.0, 0.0); // Directional light source
            vec3 light_color = vec3(1.0); // Light color

            // Transform normal map to world space
            vec3 tangent = normalize(in_tangent);
            vec3 bitangent = normalize(in_bitangent);
            vec3 normal = normalize(in_normal);

            mat3 TBN = mat3(tangent, bitangent, normal);
            vec3 world_normal = normalize(TBN * vec3(1.0));

            // Kajiya-Kay lighting
           vec3 irradiance = kajiya_kay(tangent, bitangent, world_normal, normalize(light_direction),
                                    normalize(out_view_direction), albedo, light_color, random2);

            // Apply ambient lighting
            vec3 final_color = fur_deepness * (irradiance + vec3(ambient));

            // Apply tone mapping and gamma correction
            final_color = sRGBToLinear(vec4(final_color, 1.0)).rgb;
            final_color = Aces(final_color); // HDR tone mapping
            out_color = LinearTosRGB(vec4(final_color, 1.0));
        }
        else
        discard;
    }
    else
    discard;
}