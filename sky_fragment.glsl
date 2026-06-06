#version 450

in vec2 vTexCoord;
layout(location = 0) out vec4 output_color;

uniform mat4 inv_view_proj;
uniform float u_time;

float hash2(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

void main()
{
    vec4 clipPos = vec4(vTexCoord * 2.0 - 1.0, 1.0, 1.0);
    vec4 worldPos = inv_view_proj * clipPos;
    vec3 dir = normalize(worldPos.xyz / worldPos.w);

    vec3 color = vec3(0.0);

    vec2 grid1 = vec2(atan(dir.z, dir.x), asin(dir.y)) * 50.0;
    vec2 cell1 = floor(grid1);
    vec2 uv1 = fract(grid1) - 0.5;
    float rnd1 = hash2(cell1);
    if (rnd1 > 0.97)
    {
        vec2 offset1 = vec2(hash2(cell1 + 0.1), hash2(cell1 + 0.2)) - 0.5;
        float dist1 = length(uv1 - offset1 * 0.8);
        float bright1 = 1.0 - smoothstep(0.0, 0.07, dist1);
        float twinkle1 = 0.75 + 0.25 * sin(u_time * 2.0 + rnd1 * 314.0);
        color += bright1 * twinkle1 * vec3(0.9, 0.9, 1.0);
    }

    vec2 grid2 = vec2(atan(dir.z, dir.x), asin(dir.y)) * 120.0 + vec2(3.7, 1.3);
    vec2 cell2 = floor(grid2);
    vec2 uv2 = fract(grid2) - 0.5;
    float rnd2 = hash2(cell2 + 50.0);
    if (rnd2 > 0.96)
    {
        vec2 offset2 = vec2(hash2(cell2 + 0.5), hash2(cell2 + 0.6)) - 0.5;
        float dist2 = length(uv2 - offset2 * 0.8);
        float bright2 = 1.0 - smoothstep(0.0, 0.04, dist2);
        float twinkle2 = 0.75 + 0.25 * sin(u_time * 3.0 + rnd2 * 271.0);
        color += bright2 * twinkle2 * mix(vec3(1.0, 0.85, 0.7), vec3(0.7, 0.85, 1.0), rnd2);
    }

    output_color = vec4(color, 1.0);
}
