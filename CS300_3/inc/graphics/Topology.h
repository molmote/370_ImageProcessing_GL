#ifndef H_TOPOLOGY
#define H_TOPOLOGY

namespace Graphics
{
  // This class represents primitive types for a mesh, also known as the mesh's
  // topology. The two topologies initially supported by the framework are
  // LINES and TRIANGLES. If you decide to add more topologies,
  // IndexBufferObject should work if you follow the rule below, but you will
  // also need to change the render code within VertexArrayObject::Render. It
  // should automatically work everywhere else, though (assuming you construct
  // your IndexBufferObject instances with your new topology and initialize it
  // correctly).
  enum class Topology
  {
    // The value of these are the number of indices needed to make up that type
    // of primitive. This value is used in IndexBufferObject. See
    // IndexBufferObject.h for more information.
    LINES = 2, TRIANGLES = 3
  };
}

#endif
