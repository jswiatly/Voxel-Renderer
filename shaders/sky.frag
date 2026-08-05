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
    for (int i = 0; i < 4; ++i) {
        n += noise2(p) * amp;
        amp *= 0.5;
        p = rot * p * 2.13;
    }
    return n;
}

void main() {
    vec3 ray = normalize(vRayDir);
    vec3 sun = normalize(ubo.sunDir.xyz);
    
    vec3 horizon = vec3(0.85, 0.75, 0.65) * ubo.sunColor.rgb;
    vec3 zenith = vec3(0.15, 0.35, 0.75) * ubo.sunColor.rgb;
    vec3 col = mix(horizon, zenith, sqrt(max(ray.y, 0.0001)));

    float sunCos = max(dot(ray, sun), 0.0);
    float disc = smoothstep(0.9998, 0.9999, sunCos);
    float halo = pow(sunCos, 48.0) * 0.4; 

    float aboveHorizon = smoothstep(-0.15, 0.05, sun.y);
    col += ubo.sunColor.rgb * (disc * 5.0 + halo) * aboveHorizon;

    float night = 1.0 - smoothstep(-0.2, 0.1, sun.y);

    if (night > 0.0 && ray.y > -0.05) {
        float skyGlow = smoothstep(0.0, 0.1, ray.y);
        vec3 moonDir = -sun;
        float moonCos = max(dot(ray, moonDir), 0.0);
        float moonDiscMask = smoothstep(0.9990, 0.9995, moonCos);

        vec2 cs = sun.xy;
        float csLen = max(length(cs), 0.0001);
        cs /= csLen;
        mat3 starRot = mat3(cs.x, cs.y, 0.0, -cs.y, cs.x, 0.0, 0.0, 0.0, 1.0);
        vec3 sray = starRot * ray;

        vec3 bandN = normalize(vec3(1.0, 0.4, 0.3));
        vec3 bu = normalize(cross(bandN, vec3(0.0, 1.0, 0.0)));
        vec3 bv = cross(bandN, bu);
        
        float bandDist = dot(sray, bandN);
        float bandAtten = exp(-abs(bandDist) * 5.0);

        if (bandAtten > 0.05 && moonDiscMask < 1.0) {
            vec2 mwUv = vec2(atan(dot(sray, bv), dot(sray, bu)), bandDist * 1.5) * 4.0;
            
            vec2 q = vec2(fbm(mwUv), fbm(mwUv + vec2(5.2, 1.3)));
            float gn = fbm(mwUv + 3.0 * q);

            float rift = 1.0 - 0.75 * exp(-abs(bandDist - 0.15 * (q.x - 0.5)) * 25.0);
            float core = exp(-distance(sray, bu) * 3.5);

            float lengthMod = 0.6 + 0.8 * noise2(mwUv * 1.5 + vec2(7.7));
            float band = bandAtten * exp(abs(bandDist) * 5.0 * (0.8 * lengthMod - 0.6));
            float milky = band * rift * (0.2 + 1.2 * gn) * lengthMod;
            
            vec3 milkyCol = mix(vec3(0.08, 0.10, 0.18), vec3(0.35, 0.22, 0.15), core);
            milkyCol = mix(milkyCol, vec3(0.15, 0.11, 0.22), 0.5 * q.y);
            col += milkyCol * (milky + 1.5 * core * band * rift) * night * skyGlow * (1.0 - moonDiscMask);
        }

        float auroraFade = smoothstep(0.0, 0.3, ray.y) * smoothstep(0.8, 0.3, ray.y);
        if (auroraFade > 0.0 && moonDiscMask < 1.0) {
            vec2 aUv = ray.xz / max(ray.y, 0.001);
            float auroraTime = ubo.sunDir.x * 4.0 + ubo.sunDir.z * 2.0;
            aUv += auroraTime * 0.15;
            
            vec2 warp = vec2(fbm(aUv * 0.3), fbm(aUv * 0.3 + 12.34));
            aUv += warp * 1.5;
            
            float aNoise = fbm(aUv * 0.6);
            float ribbon = abs(aNoise - 0.5); 
            float curtain = smoothstep(0.15, 0.0, ribbon); 
            
            float streaks = fbm(vec2(ray.x * 15.0, ray.y * 30.0 + auroraTime));
            curtain *= (0.4 + 0.6 * streaks);
            
            vec3 colorBottom = vec3(0.0, 1.0, 0.4);
            vec3 colorTop = vec3(0.8, 0.1, 0.7);
            vec3 auroraColor = mix(colorBottom, colorTop, smoothstep(0.1, 0.6, ray.y));
            
            col += auroraColor * curtain * auroraFade * night * skyGlow * (1.0 - moonDiscMask) * 1.5;
        }


        if (moonDiscMask < 1.0 && ray.y > 0.1) {
            float meteorTime = (ubo.sunDir.x + ubo.sunDir.z) * 1500.0; 
            
            vec2 skyUv = vec2(atan(sray.z, sray.x), asin(sray.y));
            vec2 slantedUv = mat2(0.8, 0.6, -0.6, 0.8) * skyUv * 20.0;
            slantedUv.x += meteorTime;
            
            vec2 mId = floor(slantedUv);
            float mHash = hash21(mId);
            
            if (mHash > 0.985) {
                vec2 mF = fract(slantedUv);
                
                float mLine = smoothstep(0.1, 0.0, abs(mF.y - 0.5));
                float mHead = smoothstep(0.0, 0.8, mF.x) * smoothstep(1.0, 0.9, mF.x);
                
                float mSparkle = fract(meteorTime * 15.0 + mHash * 100.0);
                float mIntensity = mLine * mHead * (0.6 + 0.4 * mSparkle);
                
                vec3 meteorColor = mix(vec3(0.6, 0.8, 1.0), vec3(0.5, 1.0, 0.7), hash21(mId + 1.0));
                
                col += meteorColor * mIntensity * 12.0 * night * skyGlow;
            }
        }

        if (moonDiscMask < 1.0) {
            vec3 cell = floor(sray * 150.0);
            float h = hash31(cell);
            vec2 starPos = 0.2 + 0.6 * vec2(fract(h * 137.0), fract(h * 517.0));
            float d = distance(fract(sray * 150.0).xy, starPos);
            
            float star = step(0.997 - 0.015 * bandAtten, h) * smoothstep(0.6, 0.15, d) * fract(h * 1000.0);
            vec3 starColor = mix(vec3(0.8, 0.9, 1.0), vec3(1.0, 0.8, 0.6), fract(h * 777.0));
            col += starColor * star * night * skyGlow;
        }

        vec3 moonColor = vec3(0.80, 0.82, 0.85);
        float moonGlow = pow(moonCos, 200.0) * 0.2;
        col += moonColor * moonGlow * night * skyGlow;

        if (moonCos > 0.999) {
            float moonDiscDraw = smoothstep(0.9993, 0.9995, moonCos);
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
                float w = 1.0 / (1.0 + 0.8 * float(i));
                surface += (-0.30 * smoothstep(rad, rad * 0.35, dc) + 
                             0.16 * (smoothstep(rad * 1.3, rad, dc) - smoothstep(rad, rad * 0.7, dc))) 
                             * step(0.4, chh) * w;
            }
            col += moonColor * moonDiscDraw * surface * night * skyGlow;
        }
    }

    float dith = (hash21(gl_FragCoord.xy) - 0.5) / 255.0;
    outColor = vec4(col + dith, 1.0);
}