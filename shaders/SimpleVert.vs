#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 Model;

layout (std140) uniform Matrices {
    mat4 projectionMatrix;
};

void main() {
    gl_Position = projectionMatrix * Model * vec4(aPos, 1.0);
}
