#version 450

uniform sampler2D texture_sampler;
uniform sampler2D lighting_sampler;
uniform sampler2D normalmap_sampler;

in vec3 vPos;
layout(location=0) out vec4 output_color;

void main() {
    vec3 dX = dFdx(vPos);
    vec3 dY = dFdy(vPos);
    vec3 flat_normal = normalize(cross(dX, dY));

    vec3 light_dir = normalize(vec3(1.0, 1.0, 1.0)); // Lumière en haut à droite
    float diff = max(dot(flat_normal, light_dir), 0.0);

    // Couleur (Un petit vert planète)
    vec3 object_color = vec3(0.4, 0.7, 0.4);
    vec3 ambient_color = vec3(0.15, 0.15, 0.15);

    vec3 final_color = (ambient_color + diff) * object_color;

    output_color = vec4(final_color, 1.0);
}
