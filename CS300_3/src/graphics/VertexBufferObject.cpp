#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/VertexBufferObject.h"

namespace Graphics
{
  // Constructs a new VBO using all these parameters, including the actual data
  // buffer used to store vertices. Notice how the buffer size is based on the
  // size of the Vertex structure, as well as the number of vertices specified.
  VertexBufferObject::VertexBufferObject(size_t vertexCount)
    : vertexCount_(vertexCount), insertOffset_(0),
    bufferSize_(sizeof(Vertex) * vertexCount),
    buffer_(new char[bufferSize_]), glHandle_(0)
  {
  }

  VertexBufferObject::~VertexBufferObject()
  {
    // cleanup
    Destroy();
    delete[] buffer_;
  }

  bool VertexBufferObject::AddVertex(Vertex const &vertex)
  {
    // Can this VBO hold one more vertex?
    if (insertOffset_ + 1 > vertexCount_)
      return false;

    // Treat the buffer as a contiguous array of Vertices and simply copy a
    // vertex into it.
    Vertex *vertexBuffer = reinterpret_cast<Vertex *>(buffer_);
    vertexBuffer[insertOffset_++] = vertex;
    return true;
  }

  size_t VertexBufferObject::GetBufferSize() const
  {
    return bufferSize_;
  }

  void VertexBufferObject::Build()
  {
    Assert(!glHandle_, "Error: trying to build already-built vertex buffer.");
    if (glHandle_)
      return;

    // glGenBuffers creates a new OpenGL buffer object with no size. It actually
    // creates an array of objects, but we are specifying just needing one.
    glGenBuffers(1, &glHandle_);
    Assert(glHandle_, "Error: failed to create vertex buffer.");
    CheckGL();
    if (!glHandle_)
      return;

    // glBufferData initializes the buffer object to be an ARRAY_BUFFER with the
    // specified size and initial data. This is why the buffer must be
    // initialized with all of its contained vertices before building the VBO.
    // GL_STATIC_DRAW hints to the driver that we are only going to initialize
    // this buffer, never modify it, and only use it to draw. It allows for a
    // potential speed boost while rendering by storing the VBO contents in
    // actual VRAM (not guaranteed).
    Bind();
    glBufferData(GL_ARRAY_BUFFER, bufferSize_, buffer_, GL_STATIC_DRAW);
    CheckGL();
  }

  void VertexBufferObject::Bind() const
  {
    // bind the buffer as a GL_ARRAY_BUFFER (can only have one of those bound
    // at a time)
    glBindBuffer(GL_ARRAY_BUFFER, glHandle_);
    CheckGL();
  }

  void VertexBufferObject::Unbind() const
  {
    // unbinds the buffer by binding the special reserved value of 0
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void VertexBufferObject::Destroy()
  {
    if (!glHandle_)
      return; // already destroyed

    // deletes an array of buffers, but we only have one to delete
    glDeleteBuffers(1, &glHandle_);
  }
}
