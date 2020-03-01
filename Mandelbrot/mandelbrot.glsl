#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout(rgba8) uniform writeonly image2D img;

float cabs(vec2 a) {
  return sqrt(a.x * a.x + a.y * a.y);
}

uniform float cxmin;
uniform float cxmax;
uniform float cymin;
uniform float cymax;
uniform int resolution;
uniform int max_iterations;
uniform usampler1D gradient;

void main() {
    vec2 c = vec2(cxmin, cymin) + 
                  gl_GlobalInvocationID.xy / vec2(imageSize(img) / resolution) * 
                  vec2(cxmax - cxmin, cymax - cymin);
    vec2 z = vec2(0.0, 0.0);

    uint iterations;
    for (iterations = 0; iterations < max_iterations && cabs(z) < 20.0; ++iterations) {
        z = vec2(
            z.x * z.x - z.y * z.y + c.x,
            z.y * z.x + z.x * z.y + c.y
        );
    }

    float esc = iterations;
    if (iterations < max_iterations) {
        esc += 1 - log(log2(cabs(z)));
    }
    esc /= max_iterations;
    
    vec3 pixel = vec3(0.1f, 0.1f, 0.1f);
    if (iterations < max_iterations) {
      if (esc > 0.5) {
        pixel = vec3(esc, 1.0f, 1.0f);
      } else {
        pixel = vec3(0.2f, 0.2 + esc * 0.8, 0.2f);
      }
    }
    vec4 to_write = vec4(pixel, 1.0);
//    to_write = texture(gradient, esc);
    imageStore(img, ivec2(gl_GlobalInvocationID.xy), to_write);
}