#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 sunDir;
    vec4 sunColor;
    vec4 camPosTime;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out float fragViewDist;

void main() {
    // Deliberately no vertical displacement. The surface quads are greedily
    // merged, so they meet at T-junctions where a corner of one quad lands in
    // the middle of another's edge - displacing vertices there would tear the
    // sheet open along visible seams. All of the ripple is per-pixel, from the
    // normal the fragment shader derives, which does not care about topology.
    vec3 world = inPosition;

    vec4 viewPos = ubo.view * vec4(world, 1.0);
    gl_Position = ubo.proj * viewPos;

    fragWorldPos = world;
    fragViewDist = length(viewPos.xyz);
}
