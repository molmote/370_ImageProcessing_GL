#version 330 core

// These two must perfectly match the structure defined in inc/graphics/Vertex.h
layout(location = 0) in vec3 vVertex;
layout(location = 1) in vec3 vNormal;

uniform mat4 ModelViewMatrix; // local->world matrix
uniform mat4 ModelViewProjectionMatrix; // local->NDC matrix [no camera support]

uniform vec4 globalAmbient;
uniform vec3 vLightAttCoef; // attenuation coefficients(c1, c2, c3)
uniform vec3 view; // view vector

uniform struct
{
	float near;
	float far;
	vec4 color;
} Fog;

const int DIRECTIONAL = 0;
const int SPOT = 1;
const int POINT = 2;

const int MaxLights = 8; // maximum possible lights this shader supports

struct Light
{
	int type;
	vec4 direction; // direction.
	vec4 position; // position. if w = 0 -> directional light.
	vec4 ambient; // ambient light cast onto objects
	vec4 diffuse; // diffuse light cast onto objects
	vec4 specular; // specular
	float spotlight_innerCos; // cos of inner angle
	float spotlight_outerCos; // cos of outer angle. if 1 -> point light
	float spotlight_falloff; // spot light fall-off value
};

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

out vec4 litFragColor;

vec4 computeLightingTerm(in int lightIdx, in vec4 n, in vec4 vtx)
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
		diffuse = l_dot_n * light.diffuse * Material.diffuse;
		vec4 r = normalize(2 * l_dot_n * n - l);
		vec4 v = normalize(vec4(view,1) - vtx);
		specular = light.specular * Material.specular * pow(max(dot(v, r), 0), Material.shininess);
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
	vec4 totalColor = Material.emissive + globalAmbient * Material.ambient;
	for (int i = 0; i < LightCount; ++i)
		totalColor += computeLightingTerm(i, worldNormal, viewVtx);
	float v_length = length(vec4(view, 1) - viewVtx);
	float s = min((Fog.far - v_length) / (Fog.far - Fog.near), 1);
	return s * totalColor + (1 - s) * Fog.color;
}

void main()
{
	// deal with position and normal in world space
	vec4 worldVtx = ModelViewMatrix * vec4(vVertex, 1);

	// vec4(vNormal, 0) because we don't want to translate a normal;
	// NOTE: this code is wrong if we support non-uniform scaling
	vec4 worldNorm = normalize(ModelViewMatrix * vec4(vNormal, 0));

	// compute the final result of passing this vertex through the transformation
	// pipeline and yielding a coordinate in NDC space
	gl_Position = ModelViewProjectionMatrix * vec4(vVertex, 1);

	// compute the contribution of lights onto this vertex and interpolate that
	// color value across the surface of the polygon
	litFragColor = computeSurfaceColor(worldNorm, worldVtx);
}