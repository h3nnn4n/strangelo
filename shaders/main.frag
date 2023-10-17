#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D tex;

void main()
{
  vec4 texCol = texture(tex, TexCoords);

  // Alpha channel contains the number of samples we took, so we divide by that
  // to get into LDR range
  texCol.rgb /= texCol.a;

  // Gamma correction
  texCol.rgb = pow(texCol.rgb, vec3(1.0 / 2.2));

  FragColor = vec4(texCol.rgb, 1.0);
}
