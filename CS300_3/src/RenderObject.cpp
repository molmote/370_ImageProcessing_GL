#include "Precompiled.h"
#include "RenderObject.h"

RenderObject::RenderObject()
{
	pos = Math::Vector3(0, 0, -1);
	eulerRotate = Math::Vector3(0, 0, 0);
	scale = Math::Vector3(1, 1, 1);
}

