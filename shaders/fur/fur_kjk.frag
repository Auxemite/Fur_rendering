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
layout(location = 6) in vec3 in_view_direction;
#if INSTANCING == 1
layout(location = 7) in float shell_rank;
#else
uniform float shell_rank;
#endif

layout(binding = 0) uniform sampler2D in_texture;
layout(binding = 1) uniform sampler2D in_normal_texture;
layout(binding = 0) uniform Data {
    FrameData frame;
};

uniform float fur_density;
uniform float base_thickness;
uniform float tip_thickness;
uniform float hair_min_length;
uniform float hair_max_length;

uniform float fur_lighting;
uniform float roughness;
uniform float metaless;
uniform float ambient;
uniform float ambient_occ;

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

float StrandSpecular(vec3 T, vec3 V, vec3 L, float exponent)
{
    vec3 H = normalize(L + V);
    float dotTH = dot(T, H);
    float sinTH = sqrt(1.0 - dotTH * dotTH);
//    float dirAtten = smoothstep(-1.0, 0.0, dot(T, H));

//    return dirAtten * pow(sinTH, exponent);
    return pow(sinTH, exponent);
}

float SinSpec(vec3 T, vec3 L)
{
    float dotTL = dot(T, L);
    return sqrt(1.0 - dotTL * dotTL);
}

// Kajiya-Kay shading model function
vec3 kajiya_kay(vec3 T, vec3 B, vec3 N, vec3 L,
            vec3 V, vec3 albedo, vec3 light_color)
{
    // Compute the effective normal
    vec3 strand_normal = normalize(L - T * dot(T, L));
    vec3 H = normalize(L + V);

    // Diffuse term
//    vec3 h = normalize(lightVec + viewVec);
//    float ks = fresnelSchlick(rdot(h, viewVec), 0.04 + metaless);
//    vec3 diffuse = (1.0 - ks) * (texture(in_texture, in_uv)).xyz * vec3(0.1, 0.0, 0.0) * rdot(normal, lightVec) / pi;
    float diffuse = max(dot(strand_normal, L), 0.0);
//    diffuse = 0.0;

    // Specular term
    float variation = sin(in_normal.x) * sin(in_normal.y) * sin(in_normal.z) * pi;
    variation = variation * 0.5 + 0.5;
    float spec = dot(B, L) * dot(B, V) + SinSpec(B, L) * SinSpec(B, V);
    B *= variation;
    float spec2 = dot(B, L) * dot(B, V) + SinSpec(B, L) * SinSpec(B, V);
    vec3 specular = metaless * vec3(pow(spec, 100.0 / (roughness + 0.001)));
    specular += vec3(pow(spec2, 10.0 / (roughness + 0.001))) * albedo;

//    vec3 specular = metaless * vec3(1.0) * StrandSpecular(T, V, L, 100.0 / (roughness + 0.001));

//    float specular = metaless * pow(max(dot(strand_normal, half_vector), 0.0), 100.0 / (roughness + 0.001));
//    specular += 0.1 * pow(max(dot(strand_normal, H), 0.0), 1.0 / (roughness + 0.001));
//    specular = 0.0;

    // Combine lighting
    vec3 color = (diffuse * albedo + specular * light_color) * light_color;
    return color;
}

void main()
{
    vec2 uv = in_uv * fur_density;
    vec2 uv_fract = fract(uv) * 2.0 - 1.0;
    ivec2 seed = ivec2(uv.x, uv.y);
    ivec2 seed2 = ivec2(uv.y, uv.x);
    float random = rand(seed); // random value [0, 1]
    float random2 = rand(seed2) * 0.5 + 0.5; // random value [0.5, 1]
    float thickness =  base_thickness + (shell_rank / random) * (tip_thickness - base_thickness);
    float fur_deepness = pow(0.5 + 0.5 * shell_rank / thickness, fur_lighting);

    vec3 albedo = vec3(0.5, 0.0, 0.0) * random2 * texture(in_texture, in_uv).rgb;
//    albedo = vec3(0.5, 0.0, 0.0) * texture(in_texture, in_uv).rgb;

//    #ifdef TEXTURED
//        albedo = vec3(1.0) * random2 * texture(in_texture, in_uv).rgb;
//    #endif

    if (shell_rank == 0) {
        vec3 final_color = fur_deepness * (albedo + vec3(ambient)) * ambient_occ;
        final_color = sRGBToLinear(vec4(final_color, 1.0)).rgb;
        final_color = Aces(final_color); // HDR tone mapping
        out_color = LinearTosRGB(vec4(final_color, 1.0));
    }
    else if (random > shell_rank && random >= hair_min_length && random <= hair_max_length)
    {
        if (length(uv_fract) <= thickness)
        {
            // Sample textures
            vec3 light_direction = vec3(5.0, 5.0, 5.0); // Directional light source
            vec3 light_color = vec3(1.0); // Light color

            mat3 TBN = mat3(in_tangent, in_bitangent, in_normal);
            vec3 world_normal = normalize(TBN * vec3(1.0));

            // Kajiya-Kay lighting
            vec3 irradiance = kajiya_kay(in_tangent, in_bitangent, world_normal, normalize(light_direction),
                                    in_view_direction, albedo, light_color);
//            irradiance = albedo * variation;

            // Apply ambient lighting
            vec3 final_color = fur_deepness * (irradiance + vec3(ambient)) * ambient_occ;

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