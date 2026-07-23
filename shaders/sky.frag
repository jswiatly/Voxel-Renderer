#version 450

layout(binding = 0) uniform UBO { mat4 view; mat4 proj; vec4 sunDir; vec4 sunColor; } ubo;

layout(location = 0) in vec3 vRayDir;
layout(location = 0) out vec4 outColor;

float hash21(vec2 v) {
    vec3 p3 = fract(vec3(v.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float hash31(vec3 p3) {
    p3 = fract(p3 * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float noise2(vec2 p) {
    vec2 ip = floor(p);
    vec2 fp = fract(p);
    fp = fp * fp * fp * (fp * (fp * 6.0 - 15.0) + 10.0);
    float a = hash21(ip);
    float b = hash21(ip + vec2(1.0, 0.0));
    float c = hash21(ip + vec2(0.0, 1.0));
    float d = hash21(ip + vec2(1.0, 1.0));
    return mix(mix(a, b, fp.x), mix(c, d, fp.x), fp.y);
}

const mat2 octRot = mat2(0.8, -0.6, 0.6, 0.8);
float fbm(vec2 p) {
    float n = 0.0;
    float amp = 0.55;
    for (int i = 0; i < 5; ++i) {
        n += noise2(p) * amp;
        amp *= 0.5;
        p = octRot * p * 2.13;
    }
    return n;
}

float noise3(vec3 x) {
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f); 
    
    return mix(mix(mix(hash31(i + vec3(0,0,0)), hash31(i + vec3(1,0,0)), f.x),
                   mix(hash31(i + vec3(0,1,0)), hash31(i + vec3(1,1,0)), f.x), f.y),
               mix(mix(hash31(i + vec3(0,0,1)), hash31(i + vec3(1,0,1)), f.x),
                   mix(hash31(i + vec3(0,1,1)), hash31(i + vec3(1,1,1)), f.x), f.y), f.z);
}

const mat3 rot3 = mat3(
     0.00,  0.80,  0.60,
    -0.80,  0.36, -0.48,
    -0.60, -0.48,  0.64
);

float fbm3(vec3 p) {
    float n = 0.0;
    float amp = 0.55;
    for (int i = 0; i < 5; ++i) {
        n += noise3(p) * amp;
        amp *= 0.5;
        p = rot3 * p * 2.03;
    }
    return n;
}

void main() {
    vec3 ray = normalize(vRayDir);
    vec3 sun = normalize(ubo.sunDir.xyz);
    
    float t = pow(max(ray.y, 0.0), 0.5); 
    
    vec3 horizon = vec3(0.85, 0.75, 0.65) * ubo.sunColor.rgb;
    vec3 zenith = vec3(0.15, 0.35, 0.75) * ubo.sunColor.rgb;

    float cosAngle = max(dot(ray, sun), 0.0);
    float disc = pow(cosAngle, 4000.0);
    float halo = pow(cosAngle, 48.0) * 0.4; 

    vec3 col = mix(horizon, zenith, t);
    float aboveHorizon = smoothstep(-0.15, 0.05, sun.y); 
    col += ubo.sunColor.rgb * (disc * 5.0 + halo) * aboveHorizon;

    float night = 1.0 - smoothstep(-0.2, 0.1, sun.y);

    if (night > 0.0) {
        vec2 cs = sun.xy;
        float csLen = length(cs);
        cs = csLen > 0.0001 ? cs / csLen : vec2(1.0, 0.0);
        
        mat3 starRot = mat3( cs.x,  cs.y, 0.0,
                            -cs.y,  cs.x, 0.0,
                             0.0,   0.0,  1.0);
        vec3 sray = starRot * ray;

        vec3 moonDir = -sun;
        float moonCos = max(dot(ray, moonDir), 0.0);
        float moonDiscMask = smoothstep(0.9990, 0.9995, moonCos);
        float moonDiscDraw = smoothstep(0.9993, 0.9995, moonCos);

        vec3 bandN = normalize(vec3(1.0, 0.4, 0.3));
        vec3 bu = normalize(cross(bandN, vec3(0.0, 1.0, 0.0)));
        float bandDist = dot(sray, bandN);
        
        vec3 gp = sray * 6.0; 
        
        vec3 q = vec3(fbm3(gp), fbm3(gp + vec3(5.2, 1.3, 2.8)), fbm3(gp + vec3(1.1, -4.5, 3.2)));
        float gn = fbm3(gp + 3.0 * q);

        float rift = 1.0 - 0.75 * exp(-abs(bandDist - 0.15 * (q.x - 0.5)) * 25.0); 
        float core = exp(-distance(sray, bu) * 3.5); 

        float lengthMod = 0.6 + 0.8 * noise3(sray * 2.0 + vec3(7.7));
        float band = exp(-abs(bandDist) * 5.0 * (1.6 - 0.8 * lengthMod));
        float milky = band * rift * (0.2 + 1.2 * gn) * lengthMod;
        
        vec3 milkyCol = mix(vec3(0.08, 0.10, 0.18), vec3(0.35, 0.22, 0.15), core);
        milkyCol = mix(milkyCol, vec3(0.15, 0.11, 0.22), 0.5 * q.y);
        col += milkyCol * (milky + 1.5 * core * band * rift) * night * smoothstep(0.0, 0.1, ray.y) * (1.0 - moonDiscMask);

        vec3 cell = floor(sray * 150.0);
        float h = hash31(cell);
        vec3 fcell = fract(sray * 150.0);
        vec2 starPos = 0.2 + 0.6 * vec2(fract(h * 137.0), fract(h * 517.0));
        float d = distance(fcell.xy, starPos);
        
        float star = step(0.997 - 0.015 * milky, h) * smoothstep(0.6, 0.15, d) * fract(h * 1000.0);
        vec3 starColor = mix(vec3(0.8, 0.9, 1.0), vec3(1.0, 0.8, 0.6), fract(h * 777.0));
        col += starColor * star * night * smoothstep(0.0, 0.1, ray.y) * (1.0 - moonDiscMask);

        vec3 mu = normalize(cross(moonDir, vec3(0.0, 1.0, 0.0)));
        vec3 mv = cross(moonDir, mu);
        vec2 muv = vec2(dot(ray, mu), dot(ray, mv)) * 40.0;

        float surface = 0.72 + 0.45 * fbm(muv * 0.30 + 4.7);
        for (int i = 0; i < 2; ++i) {
            vec2 cm = muv * (1.0 + 1.3 * float(i));
            vec2 ic = floor(cm);
            float chh = hash21(ic + 17.0 * float(i));
            vec2 center = ic + 0.25 + 0.5 * vec2(chh, fract(chh * 91.7));
            float rad = 0.12 + 0.22 * fract(chh * 37.3);
            float dc = length(cm - center);
            float hasCrater = step(0.4, chh);
            float w = 1.0 / (1.0 + 0.8 * float(i));
            float floorDark = smoothstep(rad, rad * 0.35, dc);
            float rim = smoothstep(rad * 1.3, rad, dc) - smoothstep(rad, rad * 0.7, dc);
            surface -= 0.30 * floorDark * hasCrater * w;
            surface += 0.16 * rim * hasCrater * w;
        }
        
        float moonGlow = pow(moonCos, 200.0) * 0.2; 
        vec3 moonColor = vec3(0.80, 0.82, 0.85); 
        col += (moonColor * moonDiscDraw * surface + moonColor * moonGlow) * night * smoothstep(0.0, 0.05, ray.y);
    }

    float dith = (hash21(gl_FragCoord.xy) - 0.5) / 255.0;
    outColor = vec4(col + dith, 1.0);
}