#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 sunDir;
    vec4 sunColor;
    vec4 camPosTime;
} ubo;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in float fragViewDist;

layout(location = 0) out vec4 outColor;

float hash21(vec2 v) {
    vec3 p3 = fract(vec3(v.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float noise2(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    vec2 e = vec2(1.0, 0.0);
    return mix(mix(hash21(i), hash21(i + e.xy), f.x),
               mix(hash21(i + e.yx), hash21(i + e.xx), f.x), f.y);
}

float fbm(vec2 p) {
    float n = 0.0;
    float amp = 0.5;
    mat2 rot = mat2(0.8, -0.6, 0.6, 0.8);
    for (int i = 0; i < 3; ++i) {
        n += noise2(p) * amp;
        amp *= 0.5;
        p = rot * p * 2.13;
    }
    return n;
}

// Two noise fields scrolling against each other; the ripple normal comes from
// finite differences of this, which is cheaper than sampling a normal map and
// needs no texture.

const float WAVE_SPEED = 3.0f;

float waveHeight(vec2 p, float t) {
    t *= WAVE_SPEED;
    return fbm(p * 0.12 + vec2(t * 0.25, -t * 0.18)) + 0.5 * fbm(p * 0.31 - vec2(t * 0.40, t * 0.12));
}

void main() {
    float t = ubo.camPosTime.w;
    vec2 p = fragWorldPos.xz;

    const float EPS = 0.35;
    const float RIPPLE = 1.4;
    float h0 = waveHeight(p, t);
    float hx = waveHeight(p + vec2(EPS, 0.0), t);
    float hz = waveHeight(p + vec2(0.0, EPS), t);
    vec3 N = normalize(vec3((h0 - hx) * RIPPLE, 1.0, (h0 - hz) * RIPPLE));

    vec3 V = normalize(ubo.camPosTime.xyz - fragWorldPos);
    vec3 L = normalize(ubo.sunDir.xyz);

    // Looking straight down you see through the water; at a grazing angle it
    // turns into a mirror. Schlick drives both the reflection and the alpha.
    float fresnel = 0.02 + 0.98 * pow(1.0 - clamp(dot(N, V), 0.0, 1.0), 5.0);

    // Same gradient the skybox uses, so reflections track the time of day.
    vec3 R = reflect(-V, N);
    vec3 horizon = vec3(0.85, 0.75, 0.65) * ubo.sunColor.rgb;
    vec3 zenith = vec3(0.15, 0.35, 0.75) * ubo.sunColor.rgb;
    vec3 skyColor = mix(horizon, zenith, sqrt(max(R.y, 0.0001)));

    float aboveHorizon = smoothstep(-0.15, 0.05, L.y);
    float specular = pow(max(dot(R, L), 0.0), 220.0) * aboveHorizon;

    float ambient = ubo.sunColor.w;
    float diffuse = max(dot(N, L), 0.0);
    vec3 deepColor = vec3(0.03, 0.12, 0.20) * (ambient + ubo.sunColor.rgb * diffuse);

    vec3 color = mix(deepColor, skyColor, fresnel) + ubo.sunColor.rgb * specular * 4.0;

    float fogStart = 1000.0;
    float fogEnd = 2048.0;
    float fogFactor = clamp((fogEnd - fragViewDist) / (fogEnd - fogStart), 0.0, 1.0);
    vec3 fogColor = vec3(0.6, 0.7, 0.85);

    if (ubo.sunDir.w < 0.5)
        fogFactor = 1.0f;

    // Distant water fades into the same fog as the terrain, and goes opaque as
    // it does so - a half-transparent horizon reads as a hole in the world.
    float alpha = mix(0.55, 0.97, fresnel);
    outColor = vec4(mix(fogColor, color, fogFactor), mix(1.0, alpha, fogFactor));
}
