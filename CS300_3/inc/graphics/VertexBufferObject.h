#ifndef H_VERTEX_BUFFER_OBJECT
#define H_VERTEX_BUFFER_OBJECT

#include "graphics/Buffer.h"
#include "graphics/Vertex.h"

namespace Graphics
{
  // This class represents a vertex array buffer object (VBO) in OpenGL. This
  // is where terminology starts to get a bit confusing, but it is very clearly
  // explained within the getting started presentation accopanying this
  // framework. This class abstracts a buffer object (similar to
  // IndexBufferObject) that stores vertex information (GL_ARRAY_BUFFER). This
  // buffer stores what is known as a vertex array inside of an OpenGL buffer.
  // This is not to be confused with a Vertex Array Object (VAO), which is very
  // different and explained in VertexArrayObject.h. A VBO, likewise to an IBO,
  // only stored vertex information within its buffer. Consider it an array of
  // vertices that are stored directly on the GPU. The memory layout is
  // basically identical to what the vertex array looks like inside
  // TriangleMesh.
  class VertexBufferObject : public IBuffer
  {
  public:

    // Constructs a new VBO given the number of vertices it should contain. The
    // size of the underlying buffer is dictated based on the size of the Vertex
    // structure and the number of vertices. For example, if the Vertex struct
    // currently contains a position and normal, where both are vec3s, and a
    // VBO is constructed with capacity for 8 vertices, then this buffer will
    // have capacity for (sizeof(Vertex) = 24 bytes * 8) = 192 bytes of data.
    VertexBufferObject(size_t vertexCount);

    // Destroys this VBO and cleans up any CPU-side memory associated with it,
    // such as the underlying data buffer.
    virtual ~VertexBufferObject() override;

    // Retrieves the number of vertices storable within this VBO.
    inline size_t GetVertexCount() const { return vertexCount_; }

    // Adds a vertex to this VBO, if there is capacity for it. This copies the
    // data of the vertex directly into the VBO. It returns true if there was
    // room for the vertex, or false if the VBO has been filled up.
    bool AddVertex(Vertex const &vertex);

    virtual size_t GetBufferSize() const override;
    virtual void Build() override;
    virtual void Bind() const override;
    virtual void Unbind() const override;
    virtual void Destroy() override;

  private:
    // Don't implement either. Disallow copying of this object.
    VertexBufferObject(VertexBufferObject const &) = delete;
    VertexBufferObject &operator=(VertexBufferObject const &) = delete;

    size_t vertexCount_, insertOffset_, bufferSize_;
    char *buffer_;
    unsigned int glHandle_; /* OpenGL handle to the VBO instance. */
  };
}

#endif