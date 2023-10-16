#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D tex;

void main()
{
  vec3 texCol = texture(tex, TexCoords).rgb;

  if (TexCoords.x < 0.5) {
    FragColor = vec4(TexCoords.yx, 0.0, 1.0);
  } else {
    FragColor = vec4(texCol, 1.0);
  }
}
