#ifndef H_VERTEX
#define H_VERTEX

#include "math/Vector3.h"

// WARNING: AS YOU CHANGE ANYTHING IN THIS FILE, BE AWARE THAT ONE CHANGE IN
// Vertex WILL REQUIRE CHANGES IN AttributeElementSizes AND
// AttributeElementCounts AS WELL.
namespace Graphics
{
  // This needs to be updated whenever Vertex is updated. It needs to contain
  // the same number of elements as Vertex contains fields and the sizes of
  // each of the respective fields in the Vertex struct need to be listed here.
  // This is used by VertexArrayObject to figure out the offsets to each of
  // the vertex attributes.
  static size_t AttributeElementSizes[] = {
    sizeof(Math::Vector3), // vVertex
    sizeof(Math::Vector3), // vNormal
	sizeof(Math::Vector3), // vTangent
	sizeof(Math::Vector3), // vBitangent
	sizeof(Math::Vector2) // vUV
  };

  // This needs to have the same number of elements as the AttributeElementSizes
  // array. Each number in this array represents the number of ELEMENTS stored
  // inside each respective ATTRIBUTE. For example, vVertex in GLSL may be
  // defined as a vec3. This means it has 3 float values, so we list 3 for the
  // first element of this array. If the second attribute of the Vertex is a
  // UV coordinate and defined in GLSL as 'vec2 vTexCoord', then we would put a
  // 2 for the second value of this array, and so on.
  static int AttributeElementCounts[] = { 3, 3, 3, 3, 2 };

  // The number of attributes stored within the Vertex. This is computed by the
  // compiler and does not need to be changed manually. It should always be
  // correct if the AttributeElementSizes and AttributeElementCounts arrays are
  // being properly updated to match Vertex, and vice versa.
  static size_t const AttributeCount = sizeof(AttributeElementCounts)
    / sizeof(*AttributeElementCounts);

  // This is the critical data structure of the framework. It is used directly
  // by both TriangleMesh and VertexBufferObject. You will be changing this
  // structure often. It needs to represent exactly the same structure as the
  // vertex being inputted into the vertex shader in GLSL. This means the data
  // type needs to match, the number of attributes, and the order of attributes
  // corresponding with their locations defined in GLSL. Location 0 should
  // always be the vertex; OpenGL expects that. Any other locations should be
  // consecutive and after vertex.
  struct Vertex
  {
    Math::Vector3 vertex; /* layout(location = 0) in vec3 vVertex; */
	Math::Vector3 normal; /* layout(location = 1) in vec3 vNormal; */
	Math::Vector3 tangent; /* layout(location = 2) in vec3 vTangent; */
	Math::Vector3 bitangent; /* layout(location = 3) in vec3 vBitangent; */
	Math::Vector2 uv; /* layout(location = 4) in vec3 vUV; */


    Vertex() : vertex(0.f), normal(0.f) { }
    explicit Vertex(Math::Vector3 const &_vertex)
      : vertex(_vertex), normal(0.f), tangent(0.f), bitangent(0.f), uv(0.f) { }
  };
}

#endif
