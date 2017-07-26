#version 330 core 

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vVertex;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vNormal;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 ModelViewProjectionMatrix;
uniform int shaderflag;


// represents material properties of the surface passed by the application
uniform struct
{
	vec4 ambient; // ambient color  of the surface/how much ambient light to absorb
	vec4 diffuse; // diffuse color of the surface/how much diffuse light to absorb
	vec4 emissive; // emissive
	vec4 specular; // specular
	float shininess; // shinniss of specular
} Material;

struct Light
{
	int type;
	vec4 direction; // direction.
	vec4 position; // position. if w = 0 -> directional light.
	vec4 ambient; // ambient light cast onto objects
	vec4 diffuse; // diffuse light cast onto objects
	vec4 specular; // specular
	vec4 spotlight_dir; // spotlight direction
	float spotlight_innerCos; // cos of inner angle
	float spotlight_outerCos; // cos of outer angle. if 1 -> point light
	float spotlight_falloff; // spot light fall-off value
};

const int DIRECTIONAL = 0;
const int SPOT = 1;
const int POINT = 2;
const int MaxLights = 8; // maximum possible lights this shader supports

uniform Light Lights[MaxLights]; // support UP TO 8 lights
uniform vec3 eye;

uniform mat4 ModelViewMatrix; // local->world matrix
void main(){

	// Output position of the vertex, in clip space : ModelViewProjectionMatrix * position
	gl_Position =  ModelViewProjectionMatrix * vec4(vVertex,1);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
	// N is normal vector, L is light vector, H is halfway vector (eye/light)
	
    if ((shaderflag & int(512)) == int(512))
    {
	vec4 worldVtx = ModelViewMatrix * vec4(vVertex, 1);
		vec3 halfVec = normalize(Lights[0].position+eye).xyz;
		vec3 lightPos = Lights[0].position.xyz - worldVtx.xyz;
		UV = (Material.diffuse * dot(vNormal, lightPos) + 
			 Material.specular * dot(vNormal, halfVec)).xy;
	}
}
 