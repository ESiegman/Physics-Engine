#version 330 core
out vec4 FragColor;

in vec2 v_fragCoord;

uniform vec3 u_color;
uniform bool u_isSphere;

void main()
{
    if (!u_isSphere) {
        float dist = length(v_fragCoord);
        if(dist > 0.5) {
            discard;
        }
    }
    FragColor = vec4(u_color, 1.0);
}
