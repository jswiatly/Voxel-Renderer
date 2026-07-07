#version 450

layout(binding = 0) uniform UBO { mat4 view; mat4 proj; vec4 sunDir; vec4 sunColor; } ubo;


layout(location = 0) in vec3 vRayDir;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 ray = normalize(vRayDir);

    float t = pow(max(ray.y, 0.0), 0.4);
    vec3 horizon = vec3(0.90, 0.80, 0.70) * ubo.sunColor.rgb;
    vec3 zenith = vec3(0.25, 0.45, 0.85) * ubo.sunColor.rgb;


    vec3 sun = normalize(ubo.sunDir.xyz);
    float cosAngle = max(dot(ray, sun), 0.0);
    
    float disc = pow(cosAngle, 4000.0);
    float halo = pow(cosAngle, 32.0) * 0.3;

    vec3 col = mix(horizon, zenith, t);
    float aboveHorizon = smoothstep(-0.1, 0.0, sun.y);
    col += ubo.sunColor.rgb * (disc * 4.0 + halo) * aboveHorizon;


    float night = 1.0 - smoothstep(-0.15, 0.05, sun.y);
    vec3 cell = floor(ray * 150.0);
    float h = fract(sin(dot(cell, vec3(127.1, 311.7, 74.7))) * 43758.5453);
    float star = step(0.998, h) * fract(h * 1000.0);
    col += vec3(star) * night * smoothstep(0.0, 0.1, ray.y);


    vec3 moonDir = -sun;
    float moonCos = max(dot(ray, moonDir), 0.0);
    float moonDisc = smoothstep(0.9993, 0.9995, moonCos);

    vec3 mu = normalize(cross(moonDir, vec3(0.0, 1.0, 0.0)));
    vec3 mv = cross(moonDir, mu);
    vec2 muv = vec2(dot(ray, mu), dot(ray, mv)) * 40.0;

    vec2 cell2 = floor(muv);
    float ch = fract(sin(dot(cell2, vec2(127.1, 311.7))) * 43758.5453);

    float moonGlow = pow(moonCos, 256.0) * 0.15;
    vec3 moonColor = vec3(0.75, 0.78, 0.82);
    col += (moonColor * moonDisc + moonColor * moonGlow) * night * smoothstep(0.0, 0.05, ray.y);

    outColor = vec4(col, 1.0);
}