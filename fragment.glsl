#version 450

uniform sampler2D texture_sampler;
uniform sampler2D lighting_sampler;
uniform sampler2D normalmap_sampler;

in vec3 vPos;
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

    vec3 ocean = vec3(0.05, 0.2, 0.6);
    vec3 sand  = vec3(0.8, 0.7, 0.3);
    vec3 grass = vec3(0.2, 0.5, 0.2);
    vec3 rock  = vec3(0.3, 0.3, 0.3);
    vec3 snow  = vec3(0.9, 0.9, 0.9);

    vec3 object_color = ocean;

    object_color = mix(object_color, sand, smoothstep(0.0, 0.05, heightPercent));
    object_color = mix(object_color, grass, smoothstep(0.05, 0.15, heightPercent));
    object_color = mix(object_color, rock, smoothstep(0.4, 0.6, heightPercent));
    object_color = mix(object_color, snow, smoothstep(0.7, 0.85, heightPercent));

    vec3 ambient_color = vec3(0.15, 0.15, 0.15);
    vec3 final_color = (ambient_color + diff) * object_color;

    output_color = vec4(final_color, 1.0);
}
