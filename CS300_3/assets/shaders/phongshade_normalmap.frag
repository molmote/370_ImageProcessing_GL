#version 330 core

#define pi 3.141592653589793238462643383279

in vec4 viewPos;
in vec4 viewNormal;
in vec4 viewTangent;
in vec4 viewBitangent;
in vec2 UV;

uniform vec4 globalAmbient;
uniform vec3 vLightAttCoef; // attenuation coefficients(c1, c2, c3)

uniform int useDiffuseTexture;
uniform int useSpecularTexture;
uniform int useNormalMapping;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;
uniform mat4 ModelViewMatrix; // local->world->view matrix

uniform int normalMapDebugMode;
const int NONE = 0;
const int TANGENT = 1;
const int BITANGENT = 2;
const int NORMALMAP = 3;

const vec3 view = vec3(0, 0, 0); // view position

uniform struct
{
	float near;
	float far;
	vec4 color;
} Fog;

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
uniform int LightCount; // number of lights enabled THIS ROUND

// represents material properties of the surface passed by the application
uniform struct
{
	vec4 ambient; // ambient color of the surface/how much ambient light to absorb
	vec4 diffuse; // diffuse color of the surface/how much diffuse light to absorb
	vec4 emissive; // emissive
	vec4 specular; // specular
	float shininess; // shinniss of specular
} Material;

const float sqrt2 = sqrt(2.0);

vec4 computeLightingTerm(in int lightIdx, in vec4 viewspaceNormal, in vec4 viewspaceViewVec, in vec4 viewspacePos, in vec4 diffuseColor, in float shininess)
{
	Light light = Lights[lightIdx];
	vec4 viewspaceLightVec;
	float att;
	if (light.type == DIRECTIONAL)
	{
		viewspaceLightVec = -normalize(light.direction);
		att = 1;
	}
	else
	{
		viewspaceLightVec = light.position - viewspacePos;
		float l_length = length(viewspaceLightVec);
		viewspaceLightVec = normalize(viewspaceLightVec);
		att = 1 / (vLightAttCoef.x + vLightAttCoef.y * l_length + vLightAttCoef.z * pow(l_length, 2));
	}

	vec4 ambient = light.ambient * Material.ambient;
	vec4 diffuse = vec4(0, 0, 0, 0);
	vec4 specular = vec4(0, 0, 0, 0);
	float spotlightEffect = 1;

	float l_dot_n = dot(viewspaceNormal, viewspaceLightVec);
	if (l_dot_n > 0)
	{
		diffuse = l_dot_n * light.diffuse * diffuseColor;
		vec4 r = normalize(2 * l_dot_n * viewspaceNormal - viewspaceLightVec);
		vec4 v = normalize(viewspaceViewVec);
		specular = light.specular * Material.specular * pow(max(dot(v, r), 0), shininess);
		if (light.type == SPOT)
		{
			vec4 viewspaceLightDir = normalize(light.direction);
			float d_dot_l = dot(viewspaceLightDir, -viewspaceLightVec);
			if (d_dot_l < light.spotlight_outerCos)
				spotlightEffect = 0;
			else if (d_dot_l > light.spotlight_innerCos)
				spotlightEffect = 1;
			else
				spotlightEffect = pow((d_dot_l - light.spotlight_outerCos) / (light.spotlight_innerCos - light.spotlight_outerCos), light.spotlight_falloff);
		}
	}
	return att * (ambient + spotlightEffect * (diffuse + specular));
}

layout(location = 0) out vec4 vFragColor;

void main()
{
	vec4 normal = normalize(viewNormal);
	vec4 tangent = normalize(viewTangent);
	vec4 bitangent = normalize(viewBitangent);
	mat4 tbnToView = mat4(tangent, bitangent, normal, vec4(0,0,0,1));

	vec4 diffuse = Material.diffuse;
	if (useDiffuseTexture == 1)
		diffuse = texture(diffuseTexture, UV);

	float shininess = Material.shininess;
	if (useSpecularTexture == 1)
		shininess = texture(specularTexture, UV).r;

	vec4 viewspaceNormal = normal;
	vec4 normalmapNormal = normalize(vec4(texture(normalTexture, UV).rgb * 2 - 1, 0));
	if (useNormalMapping == 1)
		viewspaceNormal = normalize(tbnToView * normalmapNormal);

	vec4 totalColor = vec4(0, 0, 0, 1);
	if (normalMapDebugMode == NONE)
	{
		vec4 viewViewVec = normalize(-viewPos);
		totalColor = Material.emissive + globalAmbient * Material.ambient;
		for (int i = 0; i < LightCount; ++i)
			totalColor += computeLightingTerm(i, viewspaceNormal, viewViewVec, viewPos, diffuse, shininess);
		float v_length = length(viewPos);
		float s = min((Fog.far - v_length) / (Fog.far - Fog.near), 1);
		totalColor = s * totalColor + (1 - s) * Fog.color;
	}
	else if(normalMapDebugMode == TANGENT)
	{
		totalColor = vec4((tangent.xyz + 1) * 0.5, 1);
	}
	else if(normalMapDebugMode == BITANGENT)
	{
		totalColor = vec4((bitangent.xyz + 1) * 0.5, 1);
	}
	else if(normalMapDebugMode == NORMALMAP)
	{
		totalColor = vec4((normalmapNormal.xyz + 1) * 0.5, 1);
	}
	vFragColor = totalColor;
}