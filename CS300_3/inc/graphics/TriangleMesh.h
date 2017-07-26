#ifndef H_TRIANGLE_MESH
#define H_TRIANGLE_MESH

#include "framework/Utilities.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "graphics/VertexArrayObject.h"

namespace Graphics
{
  class ShaderProgram;
  enum class TextureProjectorFunction;
  struct Vertex;

  // This class represents the data structure for storing geometry data in a
  // convenient way to use directly with OpenGL. This data structure is designed
  // based on the Wavefront OBJ file format, but it is similar to a typical mesh
  // structure in a graphics engine. It should look similar to any mesh
  // structures you may have created for CS200/CS250 (especially CS250). You
  // will likely not need to change this interface much, but you may need to
  // change TriangleMesh::Build in future assignments. Most of the changes you
  // will be doing to meshes will be the Vertex itself. See Vertex.h for more
  // information on how vertices work. TriangleMeshes are intended to be easy to
  // assemble, convert to a VertexArrayObject, and render. The sample main file
  // provided with this framework demonstrates how to build a triangle mesh in
  // memory and render it.
  class TriangleMesh
  {
  public:
    // A typical triangle structure. A triangle is made up of 3 vertices,
    // therefore, 3 indices (just like the OBJ format). Bear in mind these
    // indices start at 0, instead of 1 like the format. They are direct indices
    // within the vertices_ vector stored inside the data structure. The union
    // allows the indices of a triangle to be accessed in two different ways
    // for convenience.
    struct Triangle
    {
      union
      {
        struct
        {
          u32 a, b, c;
        };
        u32 indices[3];
      };

      Triangle(u32 _a, u32 _b, u32 _c);
    };

    TriangleMesh();
    ~TriangleMesh();

    // Adds a vertex to the vertices_ array. This could be used by a mesh loader
    // or a class creating a new mesh from scratch, such as a box or sphere.
    void AddVertex(f32 x, f32 y, f32 z);

    // Adds a triangle to the triangles_ array. This could be used by a mesh
    // loader or a class creating a new mesh from scratch, such as a box or
    // sphere.
    void AddTriangle(u32 a, u32 b, u32 c);

	void SetTextureMappingType(TextureProjectorFunction type) { textureMappingType_ = type; }

    // This performs preprocessing on the triangle mesh, such as computing
    // normals per face and vertex; centering all vertices about the origin;
    // and normalizing vertices to live within a range of [-0.5, 0.5]. This
    // should be called by a class loading the mesh or a class creating a new
    // mesh from memory.
    void Preprocess();

    // Retrieves the number of vertices stored within the mesh.
    int GetVertexCount() const;

    // Retrieves the number of triangles stored within the mesh.
    int GetTriangleCount() const;

    // Computes the centroid of a particular triangle, given its index within
    // the array of triangles. The centroid is merely the average point of all
    // three points of the triangle. Since a triangle is always convex, this is
    // guaranteed to be within the triangle.
    Math::Vector3 GetTriangleCentroid(u32 tidx) const;

    // Gets a vertex, given an index.
    Vertex const &GetVertex(u32 vidx) const;

    Vertex &GetVertex(u32 vidx);

    // Gets a triangle, given an index.
    Triangle const &GetTriangle(u32 tidx) const;

    // Gets the index of a relative vertex to a triangle. For example, calling
    // this method as such: GetPolygonIndex(2, 1) retrieves the second vertex
    // index of triangle 2 (the third triangle of the mesh; the second vertex
    // is value 'b' stored in Triangle above).
    u32 GetPolygonIndex(u32 tidx, u8 vertex) const;

    // Gets the normal of a polygon, given its index. The normals of a polygon
    // are not guaranteed to exist until TriangleMesh::Preprocess() is caled.
    Math::Vector3 const &GetPolygonNormal(u32 tidx) const;

    // Builds a new VAO containing the information within this TriangleMesh and
    // using the specified ShaderProgram. See ShaderProgram.h and
    // VertexArrayObject.h (specifically VertexArrayObject::Build) for more
    // information. If the VAO of this mesh has already been constructed, this
    // replaces the old one with a new one, allowing the mesh to be changed
    // during runtime. This is not an efficient process to change it every
    // frame, but it would work.
    void Build(std::shared_ptr<ShaderProgram> program);

    // Renders the VAO associated with this mesh, if it has been built using
    // the TriangleMesh::Build method. See VertexArrayObject::Render for more
    // information on how rendering meshes works.
    void Render();

	void RenderVertexNormals();
	void RenderVertexTangents();
	void RenderVertexBitangents();
	void RenderFaceNormals();

	// model min
	const Math::Vector3& GetBoundMin() const {
		return minimum_;
	}
	// model max
	const Math::Vector3& GetBoundMax() const {
		return maximum_;
	}

  private:
    // Centers the mesh's vertices about the origin. This method is adaptive as
    // more vertices are added. Called by Preprocess().
    void centerMesh();

    // Normalizes the vertices within the extents [-0.5, 0.5]. This method is
    // adaptive as more vertices are added. Called by Preprocess().
    void normalizeVertices();

	void generateUV();
	void generateTBN();

    std::vector<Vertex> vertices_;
    std::vector<Triangle> triangles_;
	std::vector<Math::Vector3> triangleNormals_;
	std::vector<Math::Vector3> triangleTangents_;
	std::vector<Math::Vector3> triangleBytangents_;

    /* The VAO built off of this mesh and ready for rendering. */
    std::shared_ptr<VertexArrayObject> vertexArrayObject_;
	std::shared_ptr<VertexArrayObject> vaoVertexNormals_;
	std::shared_ptr<VertexArrayObject> vaoVertexTangents_;
	std::shared_ptr<VertexArrayObject> vaoVertexBitangents_;
	std::shared_ptr<VertexArrayObject> vaoFaceNormals_;

	Math::Vector3 minimum_;
	Math::Vector3 maximum_;
	TextureProjectorFunction textureMappingType_;
  };
}

#endif