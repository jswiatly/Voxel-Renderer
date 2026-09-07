#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out float fragViewDist;

layout(push_constant) uniform Push {
    mat4 model;
} push;

void main() {
    vec4 worldPos = push.model * vec4(inPosition, 1.0);
    vec4 viewPos = ubo.view * worldPos;
    gl_Position  = ubo.proj * viewPos;

    fragColor    = inColor;
    fragTexCoord = inTexCoord;
    fragNormal   = mat3(push.model) * inNormal;
    fragViewDist = length(viewPos.xyz);
}