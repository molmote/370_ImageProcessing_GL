#ifndef H_LIGHT
#define H_LIGHT

#include "graphics/Color.h"
#include "math/Vector4.h"
#include "math/Vector3.h"

namespace Graphics
{
  // This structure represents a light very similarly to the struct defined in
  // the vertex shader (shader.vert). The structure is carefully setup to be
  // directly copyable to the GPU. However, you will likely add a lot of code
  // to this structure in future assignments as you need to implement a more
  // sophisticated approach to lighting.
	class TriangleMesh;

	enum class LightType {
		Directional,
		Spot,
		Point,
		MAX
	};

	struct Light
	{
		// positional light
		Light();

		// setup so it can be copied right over to the GPU
		LightType type;
		Math::Vector4 position; // if w == 0, it means direction and it's directional 
		Math::Vector4 direction;
		Color ambient;
		Color diffuse;
		Color specular;
		f32 spotlightInnerAngleRad;
		f32 spotlightOuterAngleRad;
		f32 spotlightFalloff;
		std::shared_ptr<TriangleMesh> mesh;

		f32 radius;
		f32 angleRad;
		f32 distFromCenter;
	};
}

#endif
