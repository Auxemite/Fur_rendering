#version 450 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec3 in_position;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;
layout(location = 6) in vec3 in_view_direction;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_color;
layout(location = 3) out vec3 out_position;
layout(location = 4) out vec3 out_tangent;
layout(location = 5) out vec3 out_bitangent;
layout(location = 6) out vec3 out_view_direction;

void main() {
    vec3 v1 = vertexPos[1] - vertexPos[0];
    vec3 v2 = vertexPos[2] - vertexPos[0];
    vec3 normal = normalize(cross(v1, v2));

    fragColor = vertexColor[0];
    for(int i = 0; i < 3; i++) {
        vec4 positionLocal = vec4(vertexPos[i], 1.0);
        gl_Position = projection * view * model * gl_in[i].gl_Position;

        out_normal = in_normal;
        out_uv = in_uv;
        out_color = in_color;
        out_position = in_position;
        out_tangent = in_tangent;
        out_bitangent = in_bitangent;
        out_view_direction = in_view_direction;

        vNormalWS = normalize(mat3(model) * normal);
        in_positionWS = (model * positionLocal).xyz;
        ViewDirectionWS = normalize(cameraPos - in_positionWS);

        EmitVertex();
    }
    EndPrimitive();
}