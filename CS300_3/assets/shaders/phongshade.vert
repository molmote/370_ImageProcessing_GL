#version 330 core


// These two must perfectly match the structure defined in inc/graphics/Vertex.h
layout(location = 0) in vec3 vVertex;
layout(location = 1) in vec3 vNormal;

uniform mat4 ModelViewMatrix; // local->world matrix
uniform mat4 ModelViewProjectionMatrix; // local->NDC matrix [no camera support]

out vec4 smoothVertex;
out vec4 worldVtx;
out vec4 worldNorm;

void main()
{
	smoothVertex = vec4(vVertex, 1);
	// deal with position and normal in world space
	worldVtx = ModelViewMatrix * vec4(vVertex, 1);

	worldNorm = normalize(ModelViewMatrix * vec4(vNormal, 0));

	gl_Position = ModelViewProjectionMatrix * vec4(vVertex, 1);
}
