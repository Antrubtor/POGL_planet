#version 450

in vec3 position;
in float biome;
in float unclampedRadius;

uniform mat4 model_matrix;
uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;
uniform float planetRadius;
uniform float u_time;

out vec3 vPos;
out vec3 vWorldPos;
out vec3 localPos;
out float vBiome;
out float vUnclampedRadius;

void main()
{
    vec3 pos = position;
    vec3 normal = normalize(pos);

    float wave = sin(dot(normal, vec3(1.0, 0.5, 0.8)) * 8.0 + u_time * 2.0)
            + sin(dot(normal, vec3(-0.5, 1.0, 0.3)) * 5.0 + u_time * 1.5);

    float waveOffset = (unclampedRadius < planetRadius) ? wave * 0.015 : 0.0;
    pos += normal * waveOffset;

    gl_Position = projection_matrix * model_view_matrix * vec4(pos, 1.0);
    vPos = vec3(model_view_matrix * vec4(pos, 1.0));
    vWorldPos = vec3(model_matrix * vec4(pos, 1.0));
    localPos = pos;
    vBiome = biome;
    vUnclampedRadius = unclampedRadius;
}
