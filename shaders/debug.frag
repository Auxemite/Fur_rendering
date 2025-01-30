#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_texture;

uniform uint render_mode;

void main()
{
    const ivec2 coord = ivec2(gl_FragCoord.xy);
    const vec3 color = texelFetch(in_texture, coord, 0).rgb;
    out_color = vec4(color, 1.0);

    if (render_mode == 3)
    {
        const float depth = pow(color.r, 0.35);
        out_color = vec4(vec3(depth), 1.0);
    }
    else {
        out_color = vec4(color, 1.0);
    }
}
