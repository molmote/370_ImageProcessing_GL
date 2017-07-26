#include "Precompiled.h"
#include "framework/Debug.h"
#include "framework/Utilities.h"
#include "graphics/IndexBufferObject.h"

namespace
{
  // Default type used to store indices is a 32-bit integral value.
  typedef u32 IndexType;
}

namespace Graphics
{
  // Size of the default type used to store indices.
  size_t const IndexBufferObject::DefaultIndexSize = sizeof(IndexType);

  // Constructs a new IBO given all of these parameters, including the actual
  // data buffer used to store the indices. IBOs are fixed in size and this
  // framework does not allow resizing them.
  IndexBufferObject::IndexBufferObject(Topology indexType,
    size_t primitiveCount) : topology_(indexType),
    indexCount_(static_cast<int>(indexType) * primitiveCount),
    insertOffset_(0), bufferSize_(DefaultIndexSize * indexCount_),
    buffer_(new char[bufferSize_]), glHandle_(0)
  {
  }

  IndexBufferObject::~IndexBufferObject()
  {
    // cleanup
    Destroy();
    delete[] buffer_;
  }

  bool IndexBufferObject::AddLine(int fromIndex, int toIndex)
  {
    // can the buffer fit 2 more indices?
    if (insertOffset_ + 2 > indexCount_)
      return false;

    // store the indices using the default index type
    IndexType *indexBuffer = reinterpret_cast<IndexType *>(buffer_);
    indexBuffer[insertOffset_++] = static_cast<IndexType>(fromIndex);
    indexBuffer[insertOffset_++] = static_cast<IndexType>(toIndex);
    return true;
  }

  bool IndexBufferObject::AddTriangle(int indexA, int indexB, int indexC)
  {
    // can the buffer fit 3 more indices?
    if (insertOffset_ + 3 > indexCount_)
      return false;

    // store the indices using the default index type
    IndexType *indexBuffer = reinterpret_cast<IndexType *>(buffer_);
    indexBuffer[insertOffset_++] = static_cast<IndexType>(indexA);
    indexBuffer[insertOffset_++] = static_cast<IndexType>(indexB);
    indexBuffer[insertOffset_++] = static_cast<IndexType>(indexC);
    return true;
  }

  size_t IndexBufferObject::GetBufferSize() const
  {
    return bufferSize_;
  }

  void IndexBufferObject::Build()
  {
    Assert(!glHandle_, "Error: trying to build already-built index buffer.");
    if (glHandle_)
      return;

    // glGenBuffers creates a new buffer object in OpenGL with no size
    glGenBuffers(1, &glHandle_);
    Assert(glHandle_, "Error: failed to create index buffer.");
    CheckGL();
    if (!glHandle_)
      return;

    // This uploads the current buffer to the bound buffer object; this must be
    // done after all of the indices have been added to the buffer. It also
    // initializes the buffer object to be an ELEMENT_ARRAY_BUFFER.
    // GL_STATIC_DRAW hints to the driver to optimize this buffer object for
    // rendering and not mutation, so it might store the contents of the buffer
    // directly on the graphics card (not guaranteed).
    Bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize_, buffer_, GL_STATIC_DRAW);
    CheckGL();
  }

  void IndexBufferObject::Bind() const
  {
    // binds this IBO as a GL_ELEMENT_ARRAY_BUFFER (can only have one of those
    // bound at a time, which is why it differentiates from VBO's
    // GL_ARRAY_BUFFER)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glHandle_);
    CheckGL();
  }

  void IndexBufferObject::Unbind() const
  {
    // unbinds this IBO using the special OpenGL value of 0, which means 'no
    // buffer' in this case
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void IndexBufferObject::Destroy()
  {
    if (!glHandle_)
      return; // already destroyed

    // deletes the buffer, freeing any resources consumed on the GPU
    glDeleteBuffers(1, &glHandle_);
    glHandle_ = 0;
  }
}
