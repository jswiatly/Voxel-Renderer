#version 450

layout(binding = 0) uniform UBO { mat4 view; mat4 proj; vec4 sunDir; vec4 sunColor; } ubo;


layout(location = 0) in vec3 vRayDir;
layout(location = 0) out vec4 outColor;

float hash21(vec2 v) {
    return fract(sin(dot(v, vec2(127.1, 311.7))) * 43758.5453);
}

float noise2(vec2 p) {
    vec2 ip = floor(p);
    vec2 fp = fract(p);
    fp = fp * fp * (3.0 - 2.0 * fp);
    float a = hash21(ip);
    float b = hash21(ip + vec2(1.0, 0.0));
    float c = hash21(ip + vec2(0.0, 1.0));
    float d = hash21(ip + vec2(1.0, 1.0));
    return mix(mix(a, b, fp.x), mix(c, d, fp.x), fp.y);
}

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

    vec2 cs = normalize(sun.xy);
    mat3 starRot = mat3( cs.x, cs.y, 0.0,
                    -cs.y, cs.x, 0.0,
                      0.0,  0.0, 1.0);
    vec3 sray = starRot * ray;


    float night = 1.0 - smoothstep(-0.15, 0.05, sun.y);
    vec3 cell = floor(sray * 150.0);
    float h = fract(sin(dot(cell, vec3(127.1, 311.7, 74.7))) * 43758.5453);

    // milky way: noisy band + dark dust rift + warm galactic core
    vec3 bandN = normalize(vec3(1.0, 0.4, 0.3));
    vec3 bu = normalize(cross(bandN, vec3(0.0, 1.0, 0.0)));
    vec3 bv = cross(bandN, bu);
    vec2 guv = vec2(dot(sray, bu), dot(sray, bv));

    float gn = 0.0;
    float amp = 0.6;
    vec2 gp = guv * 5.0;
    for (int i = 0; i < 3; ++i) {
        gn += noise2(gp) * amp;
        amp *= 0.5;
        gp *= 2.3;
    }

    float bandDist = dot(sray, bandN);
    float band = exp(-abs(bandDist) * 5.0);
    float rift = 1.0 - 0.7 * exp(-abs(bandDist - 0.05 * (gn - 0.5)) * 20.0);
    float core = exp(-distance(sray, bu) * 3.0);

    float milky = band * rift * (0.3 + 1.1 * gn);
    vec3 milkyCol = mix(vec3(0.10, 0.11, 0.17), vec3(0.24, 0.18, 0.12), core);
    col += milkyCol * (milky + 1.2 * core * band * rift) * night * smoothstep(0.0, 0.1, ray.y);

    vec3 fcell = fract(sray * 150.0);
vec2 starPos = 0.2 + 0.6 * vec2(fract(h * 137.0), fract(h * 517.0));
float d = distance(fcell.xy, starPos);
float star = step(0.998 - 0.010 * milky, h) * smoothstep(0.5, 0.1, d) * fract(h * 1000.0);
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
    float dith = (hash21(gl_FragCoord.xy) - 0.5) / 255.0;
    outColor = vec4(col + dith, 1.0);
}