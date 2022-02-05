// TODO(Tobi): I decided to not care about the shader thing for now

#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

V2F {
    vec3 OurColor;
    vec2 TexCoord;
}

OUT {
    vec4 FragColor
}

V2FReturn Vert(IN in) {
    V2F ret;
    ret.Position = vec4(in.Pos);
    ret.OurColor = in.Color;
    ret.TexCoord = in.TexCoord;

    return ret;
}

OUT Frag(V2F v2f) {

}