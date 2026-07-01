#version 450

layout(binding = 0) uniform UBO { mat4 view; mat4 proj; vec4 sunDir; vec4 sunColor; } ubo;
layout(location = 0) out vec3 vRayDir;

void main() {
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vec2 ndc = uv * 2.0 - 1.0;
    gl_Position = vec4(ndc, 1.0, 1.0);
    vec4 viewPos = inverse(ubo.proj) * vec4(ndc, 1.0, 1.0);
    vRayDir = mat3(inverse(ubo.view)) * (viewPos.xyz / viewPos.w);
}