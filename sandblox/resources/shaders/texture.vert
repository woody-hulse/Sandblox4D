#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 uv_in;

out vec3 uv;

void main() {

    gl_Position = vec4(position, 1.0);
    uv = uv_in;
}
