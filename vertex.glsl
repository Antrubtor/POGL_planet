#version 450

in vec3 position;
in vec3 normalFlat;
in vec3 normalSmooth;
in vec3 color;
in vec2 uv;

vec3 light_color = vec3(1.0, 1.0, 1.0);
vec3 light_position = vec3(1.0, 1.0, 5.0);

mat4 model_view_matrix = mat4(
			      0.57735, -0.33333, 0.57735, 0.00000,
			      0.00000, 0.66667, 0.57735, 0.00000,
			      -0.57735, -0.33333, 0.57735, 0.00000,
			      0.00000, 0.00000, -17, 1.00000);
mat4 projection_matrix = mat4(
			      5.00000, 0.00000, 0.00000, 0.00000,
			      0.00000, 5.00000, 0.00000, 0.00000,
			      0.00000, 0.00000, -1.00020, -1.00000,
			      0.00000, 0.00000, -10.00100, 0.00000);

out vec3 vPos;
out vec3 localPos;

void main() {
    gl_Position = projection_matrix * model_view_matrix * vec4(position, 1.0);
    vPos = vec3(model_view_matrix * vec4(position, 1.0));
    localPos = position;
}
