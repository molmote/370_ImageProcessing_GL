#version 330 core

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform float mixAmount;

in vec4 smoothModelPosition;

layout(location = 0) out vec4 vFragColor;

vec2 computeBoxUV()
{
  // pick UV based on maximal extents (this is not used in CS300 assignment 2,
  // it's merely another way to generate UVs)...only allow comps in [-1, 1]
  vec3 position = clamp(smoothModelPosition.xyz, vec3(-1), vec3(1));
  
  // find largest standard basis bias
  vec3 mag = abs(position);
  vec3 biasUVs = vec3(0.5) + 0.5 * position;
  if (mag.x > mag.y && mag.x > mag.z)
  {
    // facing pos or neg x axis; use corrected y/z for UV
    return biasUVs.yz;
  }
  else if (mag.y > mag.z)
  {
    // facing pos or neg y axis; use corrected x/z for UV
    return biasUVs.xz;
  }
  else // z is the largest
  {
    // facing pos or neg z axis; use corrected x/y for UV
    return biasUVs.xy;
  }
}

void main()
{
  vec2 uv = computeBoxUV();
  vec3 diffuse = texture(diffuseTexture, uv).rgb;
  vec3 specular = texture(specularTexture, uv).rgb;

  vFragColor = vec4(mix(diffuse, specular, 1 - mixAmount), 1);
}