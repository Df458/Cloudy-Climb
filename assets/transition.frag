#version 330
uniform sampler2D u_texture;
uniform sampler2D u_transition_texture;
uniform float u_time;
uniform vec4 u_transition_color;

in vec2 v_uv;

layout(location = 0) out vec4 f_color;

void main() {
    f_color = u_transition_color;
    float val = v_uv.y * texture2D(u_transition_texture, v_uv).r + abs((v_uv.x - 0.5f) * 0.2f);
    if(val > u_time || val < u_time - 1.5)
        f_color = texture2D(u_texture, v_uv);
}
