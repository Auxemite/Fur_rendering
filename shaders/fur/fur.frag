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

float distributionGGX(vec3 vNormalWS, vec3 lightDir, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float dotNL = rdot(lightDir, vNormalWS);
    float dotNL2 = dotNL * dotNL;

    float denom = (dotNL2 * (a2 - 1.0) + 1.0);
    return a2 / (pi * denom * denom);
}

float subGeometrySchlickGGX(float dotNV, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return dotNV / (dotNV * (1.0 - k) + k);
}

float geometrySchlickGGX(vec3 vNormalWS, vec3 lightDir, vec3 ViewDirectionWS, float roughness)
{
    float dotNL = rdot(vNormalWS, lightDir);
    float dotNV = rdot(vNormalWS, ViewDirectionWS);
    return subGeometrySchlickGGX(dotNL, roughness) * subGeometrySchlickGGX(dotNV, roughness);
}

float fresnelSchlick(float dotVH, float f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - dotVH, 5.0);
}

vec3 brdf(vec3 normal, float roughness, float metaless, vec2 in_uv, float random)
{
    vec3 irradiance = vec3(0.0);
    vec3 lightposition = vec3(5.0, 5.0, 0.0);
    vec3 lightcolor = vec3(1.0);
    float lightintensity = 5.0;
    for (int i = 0; i < 1; i++) {
        vec3 lightDir = normalize(lightposition - in_position);

        //! specular
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 h = normalize(lightDir + out_view_direction);
        float D = distributionGGX(normal, h, roughness);
        // float D = distributionGGX(normal, lightDir, roughness);
        float G = geometrySchlickGGX(normal, lightDir, out_view_direction, roughness);
        float ks = fresnelSchlick(rdot(h, out_view_direction), 0.04 + metaless);
        // float ks = fresnelSchlick(rdot(reflectDir, ViewDirectionWS), 0.04);
        float spec = G * D * ks / (4.0 * dot(normal, out_view_direction) * dot(normal, lightDir) + 1.0);

        //! diffuse
        vec3 diffuse = (1.0 - ks) * (texture(in_texture, in_uv)).xyz * random * vec3(1.0, 0.0, 0.0) * rdot(normal, lightDir) / pi;
        diffuse *= (1.0 - metaless);

        vec3 lightSample = lightcolor * lightintensity;
        irradiance += (diffuse + vec3(spec)) * lightSample * dot(normal, lightDir);
    }
    return irradiance;
}

void main()
{
    vec2 uv = in_uv * density;
    vec2 uv_fract = fract(uv) * 2.0 - 1.0;
    ivec2 seed = ivec2(uv.x, uv.y);
    ivec2 seed2 = ivec2(uv.y, uv.x);
    float random = rand(seed); // random value [0, 1]
    float random2 = rand(seed2) * 0.5 + 0.5; // random value [0, 1]
    float thickness =  base_thickness + (shell_rank / random) * (tip_thickness - base_thickness);
    float fur_deepness = pow(0.5 + 0.5 * shell_rank / thickness, fur_lighting);

    if (shell_rank == 0)
    {
        vec3 irradiance = brdf(in_normal, roughness, metaless, in_uv, 1.0) * fur_deepness;
        vec3 albedo = sRGBToLinear(vec4(irradiance + vec3(ambient * fur_deepness), 1.0)).rgb;
        albedo = Aces(albedo); // HDR
        out_color = LinearTosRGB(vec4(albedo, 1.0));
    }
    else if (random > shell_rank && random >= min_length && random <= max_length)
    {
        if (length(uv_fract) <= thickness)
        {
            vec3 irradiance = brdf(in_normal, roughness, metaless, in_uv, random2) * fur_deepness * ambient_occ;

            vec3 albedo = sRGBToLinear(vec4(irradiance + vec3(ambient * fur_deepness), 1.0)).rgb;
            albedo = Aces(albedo); // HDR
            out_color = LinearTosRGB(vec4(albedo, 1.0));
        }
        else
            discard;
    }
    else
        discard;
}

