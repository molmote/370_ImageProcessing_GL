#pragma once

#include "graphics/TriangleMesh.h"

class RenderObject {

public:
	RenderObject();

	Math::Vector3 pos;
	Math::Vector3 eulerRotate;
	Math::Vector3 scale;
	
	std::shared_ptr<Graphics::TriangleMesh> mesh;
};