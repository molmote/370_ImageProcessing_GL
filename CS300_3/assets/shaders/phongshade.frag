#version 330 core

#define pi 3.141592653589793238462643383279

in vec4 worldVtx;
in vec4 worldNorm;
in vec4 smoothVertex;

uniform vec4 globalAmbient;
uniform vec3 vLightAttCoef; // attenuation coefficients(c1, c2, c3)
uniform vec3 view; // view position


const int CYLINDRICAL = 0;
const int SPHERICAL = 1;
const int CUBIC = 2;
uniform int projectorType;
uniform int useDiffuseTexture;
uniform int useSpecularTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform float mixAmount;

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

vec2 calculateUV()
{
	vec2 uv;
	if(projectorType == CYLINDRICAL)
	{
		uv.s = clamp(atan(smoothVertex.z, smoothVertex.x) / (2*pi), -1, 1);
		if(uv.s < 0)
			uv.s += 1;
		uv.t = smoothVertex.y + 0.5;
	}
	else if(projectorType == SPHERICAL)
	{
		uv.s = clamp(atan(smoothVertex.z, smoothVertex.x) / (2*pi), -1, 1);
		if(uv.s < 0)
			uv.s += 1;
		float r = length(smoothVertex.xyz);
		uv.t = clamp(acos(smoothVertex.y/r), 0, pi) / pi;
	}
	return uv;
}

vec4 computeLightingTerm(in int lightIdx, in vec4 n, in vec4 vtx, in vec4 materialDiffuse, in vec4 materialSpecular)
{
	Light light = Lights[lightIdx];
	vec4 l;
	float att;
	if (light.type == DIRECTIONAL)
	{
		l = -normalize(light.direction);
		att = 1;
	}
	else
	{
		l = light.position - vtx;
		float l_length = length(l);
		l = normalize(l);
		att = 1 / (vLightAttCoef.x + vLightAttCoef.y * l_length + vLightAttCoef.z * pow(l_length, 2));
	}

	vec4 ambient = light.ambient * Material.ambient;

	float spotlightEffect = 1;
	float l_dot_n = dot(n, l);
	vec4 diffuse;
	vec4 specular;
	if (l_dot_n < 0)
	{
		diffuse = vec4(0, 0, 0, 0);
		specular = vec4(0, 0, 0, 0);
	}
	else
	{
		diffuse = l_dot_n * light.diffuse * materialDiffuse;
		vec4 r = normalize(2 * l_dot_n * n - l);
		vec4 v = normalize(vec4(view, 1) - vtx);
		specular = light.specular * materialSpecular * pow(max(dot(v, r), 0), Material.shininess);
		if (light.type == SPOT)
		{
			float d_dot_l = dot(light.direction, -l);
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

vec4 computeSurfaceColor(in vec4 worldNormal, in vec4 viewVtx)
{
	vec2 uv = calculateUV();

	vec4 diffuse = Material.diffuse;
	if(useDiffuseTexture == 1)
		diffuse = texture(diffuseTexture, uv);

	vec4 specular = Material.specular;
	if (useSpecularTexture == 1)
		specular = texture(specularTexture, uv);

	vec4 totalColor = Material.emissive + globalAmbient * Material.ambient;
	for (int i = 0; i < LightCount; ++i)
		totalColor += computeLightingTerm(i, worldNormal, viewVtx, diffuse, specular);
	float v_length = length(vec4(view, 1) - viewVtx);
	float s = min((Fog.far - v_length) / (Fog.far - Fog.near), 1);
	//return vec4(0,uv.t, 0, 1);
	return s * totalColor + (1 - s) * Fog.color;
}

layout(location = 0) out vec4 vFragColor;

void main()
{
	vec4 normal = normalize(worldNorm);
	vFragColor = computeSurfaceColor(normal, worldVtx);
	//vFragColor = finalColor;
}
