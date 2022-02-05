#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 Model;

layout (std140) uniform Matrices {
    mat4 projectionMatrix;
};

out vec2 TexCoord;

void main() {
    gl_Position = projectionMatrix * Model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}