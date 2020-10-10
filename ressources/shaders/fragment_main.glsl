#version 330
uniform sampler2D frameTexture;
in vec2 t;
out vec4 outColor;
void main() {
    outColor = texture2D(frameTexture, t);
}
