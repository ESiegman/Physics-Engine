#version 330 core
out vec4 FragColor;

in vec2 v_LocalPos; 

uniform vec3 u_color;

void main() {
    float distance_from_center = length(v_LocalPos);

    if (distance_from_center <= 0.5) {
        FragColor = vec4(u_color, 1.0);
    } else {
        discard;
    }
}
