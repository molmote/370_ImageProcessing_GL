#ifndef H_INDEX_BUFFER_OBJECT
#define H_INDEX_BUFFER_OBJECT

#include "graphics/Buffer.h"
#include "graphics/Topology.h"

namespace Graphics
{
  // This class is an abstraction around the OpenGL index buffer object. An
  // index buffer object is an element array (GL_ELEMENT_ARRAY_BUFFER) stored
  // within a buffer (glGenBuffers/glDeleteBuffers). Despite being referred to
  // as an element array, when it's stored within a buffer it is more commonly
  // known as an index buffer object, or IBO for short.
  // IBOs store indexes within a mesh, rather than actual vertex data. Each
  // index is used as a lookup value within the VBO (see VertexBufferObject.h
  // for more) when rendering the mesh. VertexArrayObject.h has more
  // information on how these two structures are used to render geometry.
  class IndexBufferObject : public IBuffer
  {
  public:
    // This is the size of the integral type used to store each index value. It
    // is, by default, a 32-bit integer, therefore this size can be expected to
    // be 4 bytes.
    static size_t const DefaultIndexSize;

    // Constructs a new IndexBufferObject given a topology and primitive count.
    // Primitive count represents the number of whatever primitive type will be
    // stored within the object. For example, if the topology specified is
    // Topology::TRIANGLE, then a primitiveCount of 12 indicates this IBO
    // should have enough room to store 12 triangles worth of indices. Since
    // each triangle requires 3 indices, the IBO will therefore have 48 slots
    // for indexes. If the index size is 4 bytes, then the underlying buffer
    // will have a capacity of 192 bytes for those 12 triangles.
    IndexBufferObject(Topology indexType, size_t primitiveCount);
    virtual ~IndexBufferObject() override;

    // Retrieves the topology specified for this IBO.
    inline Topology GetTopology() const { return topology_; }

    // Retrieves the number of indices being stored for each primitive. If the
    // topology specified is Topology::LINE, this returns 2. Conversely, if it
    // is Topology::TRIANGLE, this returns 3.
    inline unsigned int GetIndicesPerPrimitive() const
    {
      return static_cast<unsigned int>(topology_);
    }

    // Retrieves the number of indexes being stored within this object. The
    // number of primitives being stored can be found as follows:
    // GetIndexCount() / GetIndicesPerPrimitive().
    inline size_t GetIndexCount() const { return indexCount_; }

    // Adds a line primitive (per its indexes) to this IBO. Since a line only
    // has two vertices, only two indexes need to be added to represent that
    // line. This method returns false if the object has run out of room for
    // more indexes and cannot completely store the line.
    bool AddLine(int fromIndex, int toIndex);

    // Adds a triangle primitive (per its indexes) to this IBO. Since a
    // triangle only has three vertices, only three indexes need to be added to
    // represent that triangle. This method assumes Counterclockwise winding of
    // the vertices, so the order of indexes is critical. This method returns
    // false if the object has run out of room for more indexes and cannot
    // completely store the triangle.
    bool AddTriangle(int indexA, int indexB, int indexC); // CCW winding

    virtual size_t GetBufferSize() const override;
    virtual void Build() override;
    virtual void Bind() const override;
    virtual void Unbind() const override;
    virtual void Destroy() override;

  private:
    // Don't implement either. Disallow copying of this object.
    IndexBufferObject(IndexBufferObject const &) = delete;
    IndexBufferObject &operator=(IndexBufferObject const &) = delete;

    Topology topology_;
    size_t indexCount_, insertOffset_, bufferSize_;
    char *buffer_;
    unsigned int glHandle_; /* OpenGL handle to the IBO instance. */
  };
}

#endif
