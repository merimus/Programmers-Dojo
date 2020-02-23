#version 130
out vec4 frag_colour;

in vec2 out_uv;

uniform sampler2D myTexture;

void main() {
  frag_colour = texture(myTexture, out_uv);
}
