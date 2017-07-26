#ifndef H_MESH_LOADER
#define H_MESH_LOADER

#include "Texture.h"

namespace Graphics
{
  class TriangleMesh;

  // This class is responsible for loading new meshes from a file, using the
  // Wavefront OBJ file format. The implementation of this class is not fully
  // provided for you, so you are therefore expected to implement it for
  // assignment 1. Refer to TriangleMesh.h and the example Main file for how
  // constructing a TriangleMesh works. Bear in mind you only need to be able
  // to load the initial meshes provided (bunny.obj, horse.obj, and your own
  // crafted cube.obj), but it may be worthwhile to implement a more complete
  // implementation that can support all sorts of OBJ files. The format itself
  // is very well described here:
  //   http://en.wikipedia.org/wiki/Wavefront_.obj_file
  class MeshLoader
  {
  public:

    // Loads a new TriangleMesh from a Wavefront OBJ file, given that files
    // relative path. The path is relative to all files within assets/models
    // from the root directory of the project.
    static std::shared_ptr<TriangleMesh> LoadMesh(std::string const &objFile, TextureProjectorFunction textureMappingType);
  };
}

#endif