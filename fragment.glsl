#version 450

uniform sampler2D texture_sampler;
uniform sampler2D lighting_sampler;
uniform sampler2D normalmap_sampler;

in vec3 vPos;
in float vBiome;
layout(location=0) out vec4 output_color;

uniform float minElevation;
uniform float maxElevation;
in vec3 localPos;

in float vUnclampedRadius;
uniform float planetRadius;

void main() {
    vec3 dX = dFdx(vPos);
    vec3 dY = dFdy(vPos);
    vec3 flat_normal = normalize(cross(dX, dY));

    vec3 light_dir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(flat_normal, light_dir), 0.0);

    float landHeight = length(localPos);

    float heightPercent = (landHeight - planetRadius) / (maxElevation - planetRadius);
    heightPercent = clamp(heightPercent, 0.0, 1.0);

    float oceanDepthPercent = (planetRadius - vUnclampedRadius) / (planetRadius - minElevation);
    oceanDepthPercent = clamp(oceanDepthPercent, 0.0, 1.0);

    // Plaines
    vec3 b1_deep_ocean    = vec3(0.01, 0.05, 0.2);
    vec3 b1_shallow_ocean = vec3(0.05, 0.3, 0.6);
    vec3 b1_sand  = vec3(0.8, 0.7, 0.3);
    vec3 b1_grass = vec3(0.2, 0.5, 0.2);
    vec3 b1_rock  = vec3(0.3, 0.3, 0.3);
    vec3 b1_snow  = vec3(0.9, 0.9, 0.9);

    vec3 color_plains;
    if (vUnclampedRadius < planetRadius) {
        color_plains = mix(b1_shallow_ocean, b1_deep_ocean, smoothstep(0.0, 0.8, oceanDepthPercent));
    } else {
        color_plains = mix(b1_sand, b1_grass, smoothstep(0.0, 0.05, heightPercent));
        color_plains = mix(color_plains, b1_rock,  smoothstep(0.4, 0.6, heightPercent));
        color_plains = mix(color_plains, b1_snow,  smoothstep(0.7, 0.85, heightPercent));
    }

    // Désert
    vec3 b2_deep_ocean    = vec3(0.01, 0.1, 0.25);
    vec3 b2_shallow_ocean = vec3(0.05, 0.4, 0.5);
    vec3 b2_sand  = vec3(0.9, 0.8, 0.4);
    vec3 b2_grass = vec3(0.8, 0.6, 0.3);
    vec3 b2_rock  = vec3(0.6, 0.4, 0.3);
    vec3 b2_snow  = vec3(0.8, 0.8, 0.7);

    vec3 color_desert;
    if (vUnclampedRadius < planetRadius) {
        color_desert = mix(b2_shallow_ocean, b2_deep_ocean, smoothstep(0.0, 0.8, oceanDepthPercent));
    } else {
        color_desert = mix(b2_sand, b2_grass, smoothstep(0.0, 0.05, heightPercent));
        color_desert = mix(color_desert, b2_rock,  smoothstep(0.4, 0.6, heightPercent));
        color_desert = mix(color_desert, b2_snow,  smoothstep(0.7, 0.85, heightPercent));
    }



    vec3 object_color = mix(color_plains, color_desert, smoothstep(0.3, 0.7, vBiome));

    vec3 ambient_color = vec3(0.15, 0.15, 0.15);
    vec3 final_color = (ambient_color + diff) * object_color;

    output_color = vec4(final_color, 1.0);
}
