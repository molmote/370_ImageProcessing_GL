#version 330 core

uniform mat4 ModelViewProjectionMatrix; // local->NDC matrix [no camera support]

layout(location = 0) in vec3 vVertex;
layout(location = 1) in vec3 vNormal;

out vec4 smoothModelPosition;

void main()
{  
  smoothModelPosition = vec4(vVertex, 1);
  gl_Position = ModelViewProjectionMatrix * vec4(vVertex, 1);
}