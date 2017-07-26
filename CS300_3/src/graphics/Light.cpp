#include "Precompiled.h"
#include "graphics/Light.h"

namespace Graphics
{

	Light::Light()
	{
		type = LightType::Point;
		position = Math::Vector4(0, 0, 0, 1);
		direction = Math::Vector4(0, 0, 1, 0);
		ambient = Color::Black;
		diffuse = Color(0.8f, 0.8f, 0.8f, 1);
		specular = Color::White;
		spotlightInnerAngleRad = Math::DegToRad(15);
		spotlightOuterAngleRad = Math::DegToRad(30);
		spotlightFalloff = 1;
		radius = 0.25f;
	}
}
