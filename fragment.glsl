#version 450

in vec3 vPos;
in vec3 vWorldPos;
in vec3 localPos;
in float vBiome;
in float vUnclampedRadius;

layout(location = 0) out vec4 output_color;

uniform float minElevation;
uniform float maxElevation;
uniform float planetRadius;
uniform vec3 u_cameraPos;
uniform float u_time;

uniform vec3 u_currentPlanetCenter;
uniform float u_currentPlanetRadiusMax;

struct ReflectedPlanet {
    vec3 center;
    float radius;
};
uniform ReflectedPlanet u_reflPlanets[4];
uniform int u_numReflPlanets;

float sphereIntersect(vec3 ro, vec3 rd, vec3 center, float radius)
{
    vec3 oc = ro - center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - radius * radius;
    float h = b * b - c;
    if (h < 0.0) return -1.0;
    return -b - sqrt(h);
}

// Bruit simple pour simuler le terrain dans les reflets
float noiseHash(vec3 p)
{
    p = fract(p * vec3(127.1, 311.7, 74.7));
    p += dot(p, p + 19.19);
    return fract(p.x * p.y * p.z);
}

float smoothNoise(vec3 p)
{
    vec3 i = floor(p);
    vec3 f = fract(p);
    vec3 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(mix(noiseHash(i + vec3(0, 0, 0)), noiseHash(i + vec3(1, 0, 0)), u.x),
            mix(noiseHash(i + vec3(0, 1, 0)), noiseHash(i + vec3(1, 1, 0)), u.x), u.y),
        mix(mix(noiseHash(i + vec3(0, 0, 1)), noiseHash(i + vec3(1, 0, 1)), u.x),
            mix(noiseHash(i + vec3(0, 1, 1)), noiseHash(i + vec3(1, 1, 1)), u.x), u.y),
        u.z);
}

float fbm(vec3 p, int octaves)
{
    float val = 0.0;
    float amp = 0.5;
    float freq = 1.0;
    for (int k = 0; k < octaves; k++) {
        val += amp * smoothNoise(p * freq);
        amp *= 0.5;
        freq *= 2.0;
    }
    return val;
}

// Couleur d'une planète réfléchie avec terrain procédural approximé
vec3 planetSurfaceColor(vec3 hitPos, vec3 planetCenter, float pRadius,
    vec3 lightDir)
{
    vec3 hitNormal = normalize(hitPos - planetCenter);

    // Relief procédural approximé
    float noiseVal = fbm(hitNormal * 2.5, 5);
    float elevation = noiseVal - 0.5; // centré sur 0

    // Hauteur normalisée pour les couleurs biomes
    float hp = clamp(elevation * 4.0 + 0.3, 0.0, 1.0);

    vec3 col = mix(vec3(0.05, 0.30, 0.60), // ocean
            mix(vec3(0.80, 0.70, 0.30), // sable
                mix(vec3(0.20, 0.50, 0.20), // herbe
                    mix(vec3(0.35, 0.32, 0.28), // roche
                        vec3(0.92, 0.92, 0.95), // neige
                        smoothstep(0.75, 0.90, hp)),
                    smoothstep(0.50, 0.65, hp)),
                smoothstep(0.10, 0.25, hp)),
            smoothstep(0.0, 0.06, hp));

    float diff = max(dot(hitNormal, lightDir), 0.15);
    return col * diff;
}

// Couleur du fond étoilé
float hash2(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

vec3 skyColorInDir(vec3 dir)
{
    vec3 color = vec3(0.0);

    vec2 grid1 = vec2(atan(dir.z, dir.x), asin(dir.y)) * 50.0;
    vec2 cell1 = floor(grid1);
    vec2 uv1 = fract(grid1) - 0.5;
    float rnd1 = hash2(cell1);
    if (rnd1 > 0.97) {
        vec2 offset1 = vec2(hash2(cell1 + 0.1), hash2(cell1 + 0.2)) - 0.5;
        float dist1 = length(uv1 - offset1 * 0.8);
        float bright1 = 1.0 - smoothstep(0.0, 0.07, dist1);
        float twinkle1 = 0.75 + 0.25 * sin(u_time * 2.0 + rnd1 * 314.0);
        color += bright1 * twinkle1 * vec3(0.9, 0.9, 1.0);
    }

    vec2 grid2 = vec2(atan(dir.z, dir.x), asin(dir.y)) * 120.0
            + vec2(3.7, 1.3);
    vec2 cell2 = floor(grid2);
    vec2 uv2 = fract(grid2) - 0.5;
    float rnd2 = hash2(cell2 + 50.0);
    if (rnd2 > 0.96) {
        vec2 offset2 = vec2(hash2(cell2 + 0.5), hash2(cell2 + 0.6)) - 0.5;
        float dist2 = length(uv2 - offset2 * 0.8);
        float bright2 = 1.0 - smoothstep(0.0, 0.04, dist2);
        float twinkle2 = 0.75 + 0.25 * sin(u_time * 3.0 + rnd2 * 271.0);
        color += bright2 * twinkle2
                * mix(vec3(1.0, 0.85, 0.7), vec3(0.7, 0.85, 1.0), rnd2);
    }
    return color;
}

void main()
{
    vec3 dX = dFdx(vPos);
    vec3 dY = dFdy(vPos);
    vec3 flat_normal = normalize(cross(dX, dY));

    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(flat_normal, lightDir), 0.0);

    float landHeight = length(localPos);
    float heightPercent = (landHeight - planetRadius) / (maxElevation - planetRadius);
    heightPercent = clamp(heightPercent, 0.0, 1.0);

    float oceanDepthPercent = (planetRadius - vUnclampedRadius)
            / (planetRadius - minElevation);
    oceanDepthPercent = clamp(oceanDepthPercent, 0.0, 1.0);

    // ---- Couleurs biomes ----
    vec3 b1_deep_ocean = vec3(0.01, 0.05, 0.20);
    vec3 b1_shallow_ocean = vec3(0.05, 0.30, 0.60);
    vec3 b1_sand = vec3(0.80, 0.70, 0.30);
    vec3 b1_grass = vec3(0.20, 0.50, 0.20);
    vec3 b1_rock = vec3(0.30, 0.30, 0.30);
    vec3 b1_snow = vec3(0.90, 0.90, 0.90);

    vec3 b2_deep_ocean = vec3(0.01, 0.10, 0.25);
    vec3 b2_shallow_ocean = vec3(0.05, 0.40, 0.50);
    vec3 b2_sand = vec3(0.90, 0.80, 0.40);
    vec3 b2_grass = vec3(0.80, 0.60, 0.30);
    vec3 b2_rock = vec3(0.60, 0.40, 0.30);
    vec3 b2_snow = vec3(0.80, 0.80, 0.70);

    vec3 color_plains, color_desert;

    if (vUnclampedRadius < planetRadius)
    {
        color_plains = mix(b1_shallow_ocean, b1_deep_ocean,
                smoothstep(0.0, 0.8, oceanDepthPercent));
        color_desert = mix(b2_shallow_ocean, b2_deep_ocean,
                smoothstep(0.0, 0.8, oceanDepthPercent));
    }
    else
    {
        color_plains = mix(b1_sand, b1_grass, smoothstep(0.0, 0.05, heightPercent));
        color_plains = mix(color_plains, b1_rock, smoothstep(0.4, 0.6, heightPercent));
        color_plains = mix(color_plains, b1_snow, smoothstep(0.7, 0.85, heightPercent));

        color_desert = mix(b2_sand, b2_grass, smoothstep(0.0, 0.05, heightPercent));
        color_desert = mix(color_desert, b2_rock, smoothstep(0.4, 0.6, heightPercent));
        color_desert = mix(color_desert, b2_snow, smoothstep(0.7, 0.85, heightPercent));
    }

    vec3 object_color = mix(color_plains, color_desert,
            smoothstep(0.3, 0.7, vBiome));

    // Reflets de l'eau
    if (vUnclampedRadius < planetRadius)
    {
        // Normale perturbée par les vagues
        vec3 n = normalize(localPos);
        float w1 = sin(dot(n, vec3(1.0, 0.5, 0.8)) * 8.0 + u_time * 2.0);
        float w2 = sin(dot(n, vec3(-0.5, 1.0, 0.3)) * 5.0 + u_time * 1.5);
        vec3 t1 = normalize(cross(n, vec3(0.0, 1.0, 0.001)));
        vec3 t2 = normalize(cross(n, vec3(1.0, 0.001, 0.0)));
        vec3 waterNormal = normalize(n + t1 * w1 * 0.12 + t2 * w2 * 0.08);

        // Réflexion de base
        vec3 viewDir = normalize(u_cameraPos - vWorldPos);
        vec3 reflDir = reflect(-viewDir, waterNormal);

        // Déformation du contour du reflet avec du bruit fractal
        float cosTheta0 = max(dot(n, viewDir), 0.0);

        // Bruit de distorsion angulaire — fréquence élevée pour des arêtes sharp
        float dNoise1 = fbm(localPos * 8.0, 4) - 0.5;
        float dNoise2 = fbm(localPos * 8.0 + vec3(17.3, 5.7, 11.9), 4) - 0.5;
        vec3 noisePerp1 = normalize(cross(reflDir, vec3(0.0, 1.0, 0.001)));
        vec3 noisePerp2 = normalize(cross(reflDir, noisePerp1));

        // Amplitude de distorsion pour les pointes
        float distortAmp = mix(0.35, 0.08, cosTheta0 * cosTheta0);
        vec3 reflDirDistorted = normalize(reflDir
                    + noisePerp1 * dNoise1 * distortAmp
                    + noisePerp2 * dNoise2 * distortAmp);

        // Fond étoilés
        vec3 spaceBase = vec3(0.0, 0.005, 0.02);
        vec3 envColor = spaceBase + skyColorInDir(reflDirDistorted);

        // Reflet des planètes
        if (u_currentPlanetRadiusMax > 0.001)
        {
            float expandedRadius = u_currentPlanetRadiusMax * 1.08;
            float tSelf = sphereIntersect(vWorldPos, reflDirDistorted,
                    u_currentPlanetCenter,
                    expandedRadius);
            if (tSelf > 0.05)
            {
                vec3 hitPos = vWorldPos + reflDirDistorted * tSelf;
                vec3 selfColor = planetSurfaceColor(hitPos,
                        u_currentPlanetCenter,
                        expandedRadius * 0.95,
                        lightDir);
                envColor = max(envColor, selfColor);
            }
        }

        // Planètes voisines
        for (int i = 0; i < u_numReflPlanets; i++)
        {
            if (u_reflPlanets[i].radius < 0.001) continue;
            float t = sphereIntersect(vWorldPos, reflDir,
                    u_reflPlanets[i].center,
                    u_reflPlanets[i].radius);
            if (t > 0.0)
            {
                vec3 hitPos = vWorldPos + reflDir * t;
                vec3 pColor = planetSurfaceColor(hitPos,
                        u_reflPlanets[i].center,
                        u_reflPlanets[i].radius,
                        lightDir);
                envColor = max(envColor, pColor);
            }
        }

        // Couleur de l'eau
        vec3 deepColor = mix(b1_shallow_ocean, b1_deep_ocean,
                smoothstep(0.0, 0.8, oceanDepthPercent));

        // Fresnel
        float fresnelNoise = fbm(n * 6.0 + u_time * 0.3, 3);
        float cosTheta = max(dot(waterNormal, viewDir), 0.0);
        float fresnel = 0.15 + 0.85 * pow(1.0 - cosTheta, 2.0);
        fresnel = clamp(fresnel + (fresnelNoise - 0.4) * 0.3, 0.0, 1.0);

        object_color = mix(deepColor, envColor, fresnel);
    }

    vec3 ambient_color = vec3(0.15, 0.15, 0.15);
    vec3 final_color = (ambient_color + diff) * object_color;

    output_color = vec4(final_color, 1.0);
}
