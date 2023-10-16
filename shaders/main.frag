#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D tex;

void main()
{
  vec4 texCol = texture(tex, TexCoords);
  FragColor = vec4(texCol.r, texCol.g, texCol.b, 1.0);
}
