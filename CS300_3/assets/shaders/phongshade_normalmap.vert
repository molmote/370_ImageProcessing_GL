#version 330 core

// These two must perfectly match the structure defined in inc/graphics/Vertex.h
layout(location = 0) in vec3 vVertex;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec2 vUV;

uniform mat4 ModelViewMatrix; // local->world->view matrix
uniform mat4 ModelViewProjectionMatrix; // local->NDC matrix [no camera support]

out vec4 viewPos;
out vec4 viewNormal;
out vec4 viewTangent;
out vec4 viewBitangent;
out vec2 UV;

void main()
{
	viewPos = ModelViewMatrix * vec4(vVertex, 1);
	viewNormal = normalize(ModelViewMatrix * vec4(vNormal, 0));
	viewTangent = ModelViewMatrix * vec4(vTangent, 0);
	viewBitangent = ModelViewMatrix * vec4(vBitangent, 0);
	UV = vUV;

	gl_Position = ModelViewProjectionMatrix * vec4(vVertex, 1);
}
