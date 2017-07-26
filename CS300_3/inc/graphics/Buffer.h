#ifndef H_BUFFER
#define H_BUFFER

namespace Graphics
{
  // This is an interface representing an OpenGL buffer object. Within this
  // framework, it is only used as the parent for VertexBufferObject and
  // IndexBufferObject. Since it is a pure-virtual class, it has no
  // implementation details. It merely describes what the interface for
  // VertexBufferObject and IndexBufferObject should look like. It is not at
  // all essential in understanding the OpenGL functionality used in this
  // framework.
  class IBuffer
  {
  public:
    virtual ~IBuffer() { }

    // Retrieves the number of bytes stored within this buffer. How that data
    // is used and what it contains is completely dependent on the underlying
    // type of the object. IndexBufferObject.h and VertexBufferObject.h will
    // explain this in greater detail.
    virtual size_t GetBufferSize() const = 0;

    // Constructs a new OpenGL buffer and uploads the underlying buffer memory
    // to the graphics card, bound to that object.
    virtual void Build() = 0;

    // Binds this object for use within OpenGL. Binding an object allows its
    // data to be changed or allows it to be used within other OpenGL
    // operations, such as rendering.
    virtual void Bind() const = 0;

    // Unbinds this object, disallowing it to be used within any future OpenGL
    // operations until it is bound again.
    virtual void Unbind() const = 0;

    // Destroys the OpenGL buffer, allowing the graphics driver to cleanup any
    // graphics memory associated with this object. The destructor is
    // responsible for cleaning up any CPU-side memory, such as the allocated
    // data buffer.
    virtual void Destroy() = 0;
  };
}

#endif
