#version 130

in vec3 myVertex;
in vec2 myUV;

out vec2 out_uv;

void main() {
  gl_Position = vec4(myVertex, 1.0);
  out_uv = myUV;
}
