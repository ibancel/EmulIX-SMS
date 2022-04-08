#version 440

uniform sampler2D frameTexture;

in vec2 uv;
out vec4 outColor;

void main() {
    outColor = texture2D(frameTexture, uv);
}
