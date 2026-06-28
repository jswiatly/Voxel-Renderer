#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 sunDir;
    vec4 sunColor;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in float fragViewDist;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(ubo.sunDir.xyz);

    float diffuse = max(dot(N, L), 0.0);
    float ambient = ubo.sunColor.w;
    vec3 light = ambient + ubo.sunColor.rgb * diffuse;

    vec3 color = texture(texSampler, fragTexCoord).rgb * fragColor * light;

    float fogStart = 100.0;
    float fogEnd   = 250.0;
    float fogFactor = clamp((fogEnd - fragViewDist) / (fogEnd - fogStart), 0.0, 1.0);
    vec3  fogColor  = vec3(0.6, 0.7, 0.85);

    if (ubo.sunDir.w < 0.5)
        fogFactor = 1.0f;

    outColor = vec4(mix(fogColor, color, fogFactor), 1.0);
}