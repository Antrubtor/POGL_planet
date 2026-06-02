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

void main() {
    vec3 dX = dFdx(vPos);
    vec3 dY = dFdy(vPos);
    vec3 flat_normal = normalize(cross(dX, dY));

    vec3 light_dir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(flat_normal, light_dir), 0.0);

    float elevation = length(localPos);

    float heightPercent = (elevation - minElevation) / (maxElevation - minElevation);

    heightPercent = clamp(heightPercent, 0.0, 1.0);

    // Plaines
    vec3 b1_ocean = vec3(0.05, 0.2, 0.6);
    vec3 b1_sand  = vec3(0.8, 0.7, 0.3);
    vec3 b1_grass = vec3(0.2, 0.5, 0.2);
    vec3 b1_rock  = vec3(0.3, 0.3, 0.3);
    vec3 b1_snow  = vec3(0.9, 0.9, 0.9);

    vec3 color_plains = b1_ocean;
    color_plains = mix(color_plains, b1_sand,  smoothstep(0.0, 0.05, heightPercent));
    color_plains = mix(color_plains, b1_grass, smoothstep(0.05, 0.15, heightPercent));
    color_plains = mix(color_plains, b1_rock,  smoothstep(0.4, 0.6, heightPercent));
    color_plains = mix(color_plains, b1_snow,  smoothstep(0.7, 0.85, heightPercent));

    // Désert
    vec3 b2_ocean = vec3(0.02, 0.3, 0.5);
    vec3 b2_sand  = vec3(0.9, 0.8, 0.4);
    vec3 b2_grass = vec3(0.8, 0.6, 0.3);
    vec3 b2_rock  = vec3(0.6, 0.4, 0.3);
    vec3 b2_snow  = vec3(0.8, 0.8, 0.7);

    vec3 color_desert = b2_ocean;
    color_desert = mix(color_desert, b2_sand,  smoothstep(0.0, 0.05, heightPercent));
    color_desert = mix(color_desert, b2_grass, smoothstep(0.05, 0.15, heightPercent));
    color_desert = mix(color_desert, b2_rock,  smoothstep(0.4, 0.6, heightPercent));
    color_desert = mix(color_desert, b2_snow,  smoothstep(0.7, 0.85, heightPercent));



    vec3 object_color = mix(color_plains, color_desert, smoothstep(0.3, 0.7, vBiome));

    vec3 ambient_color = vec3(0.15, 0.15, 0.15);
    vec3 final_color = (ambient_color + diff) * object_color;

    output_color = vec4(final_color, 1.0);
}
