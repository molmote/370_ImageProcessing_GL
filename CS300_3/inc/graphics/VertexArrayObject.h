#ifndef H_VERTEX_ARRAY_OBJECT
#define H_VERTEX_ARRAY_OBJECT

#include "framework/Utilities.h"
#include "graphics/IndexBufferObject.h"
#include "graphics/VertexBufferObject.h"

namespace Graphics
{
  class ShaderProgram;

  // A Vertex Array Object (VAO) is an OpenGL 3 construct which helps simplify
  // the drawing process with VBOs and IBOs. This is a gross understatement for
  // the capabilities of a VAO, but this framework keeps its usage of them
  // simple. We use them because, with a few extra initialization steps, we are
  // able to very easily render objects with just 3 OpenGL calls (bind VAO,
  // render, unbind). For the simplification of the framework, consider a VAO to
  // be a container of a VBO, IBO, and what's known as vertex layout
  // information. Vertex layout information is described in more detail in the
  // implementation of VertexArrayObject::Build. You do not need to worry about
  // manually constructing VBOs and IBOs; you just need to construct one VAO.
  // You can then populate the IBO and VBO of the VAO with indexes and vertices,
  // then build the VAO (which, in turn, builds the VBO and IBO). It is then
  // ready to render. See VertexBufferObject.h and IndexBufferObject.h for more
  // information on VBOs and IBOs, respectively.
  class VertexArrayObject
  {
  public:

    // Constructs a new VAO (and respective VBO and IBO) given a vertex count,
    // primitive count, and topology (defaulted to triangles).
    VertexArrayObject(size_t vertexCount, size_t primitiveCount,
      Topology topology = Topology::TRIANGLES);

    // Destroys this VAO (and the underlying VBO and IBO) and cleans up any
    // resources associated with it (both on CPU and GPU).
    ~VertexArrayObject();

    // Retrieves the IBO used by this VAO.
    inline IndexBufferObject &GetIndexBufferObject() { return ibo_; }

    // Retrieves the VBO used by this VAO.
    inline VertexBufferObject &GetVertexBufferObject() { return vbo_; }

    // Builds the VAO on the GPU, as well as the VBO and IBO used by this VAO.
    // This process involves building the vertex array object, binding the
    // provided program, then setting up the vertex layout using information
    // found inside Vertex (AttributeElementSizes and AttributeElementCounts).
    // This information is persisted within the VAO. The VBO and IBO are then
    // constructed and linked to this VAO. Note that the program being specified
    // is critical for building the VAO. In a more serious graphics engine, this
    // could make a bigger difference. However, since all shader programs in
    // this framework are likely to use the same vertex layout, then which
    // program that is specified to build the VAO does not actually matter. It
    // does not necessarily have to be the same program used to render it. This
    // means one VAO instance can be rendered by all of your shader programs,
    // if they are using the same vertex attributes in their vertex shaders.
    void Build(std::shared_ptr<ShaderProgram> program);

    // Binds the VAO, readying it for rendering.
    void Bind();

    // Based on the input topology and number of indices, performs an
    // element-based rendering using the VAO. This is another way of saying that
    // it is telling OpenGL to start rendering using the VBO contained within
    // this VAO (using the vertex layout as a lookup reference, per how it is
    // defined in Build) and to use the IBO contained within the VAO to lookup
    // vertices inside the VBO. OpenGL can render without using an IBO, but we
    // are using an IBO due to it being simpler to work with in regards to our
    // TriangleMesh data structure. The VAO must be bound in order for Render
    // to work correctly.
    void Render();

    // Unbinds the VAO, disallowing it to be used for any future OpenGL calls
    // until it is bound again.
    void Unbind();

  private:
    unsigned int vertexArrayHandle_; /* OpenGL handle to the VAO instance. */

    IndexBufferObject ibo_;
    VertexBufferObject vbo_;
  };
}

#endif
