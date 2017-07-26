#ifndef H_SHADER_PROGRAM
#define H_SHADER_PROGRAM

#include "framework/Utilities.h"

namespace Math
{
  struct Matrix4;
  struct Vector4;
  struct Vector3;
}

namespace Graphics
{
  struct Color;

  // A ShaderProgram is a wrapper around the GLSL shader objects stored within
  // OpenGL. Shader programs on the GPU are lightweight applications that fill
  // in functionality within the transformation and rasterization pipelines of
  // the GPU. They essentially represent the low-level code written in CS200,
  // but now that code is being executed on the GPU itself. Since it's a
  // different architecture and processor, interacting with that application is
  // much more complicated than any CPU-side C/C++ application. This class
  // helps to simplify that interaction process, but you will want to have a
  // good understanding of how the transformation/rasterization process works
  // on the GPU before trying to understand this interface. ShaderProgram.cpp
  // goes into greater depth on what an OpenGL shader program actually is, as
  // well as the OpenGL code needed to work with it. For more information on
  // building one of these classes yourself, see:
  //   http://ogldev.atspace.co.uk/www/tutorial04/tutorial04.html
  class ShaderProgram
  {
  public:
    // Unbinds and destroys the shader program, including any CPU-side or
    // GPU-side resources associated with the program.
    ~ShaderProgram();

    // Checks whether the shader program has a uniform by a given name, such as
    // "MVP" or "LightCount."
    bool HasUniform(std::string const &name) const;

    // Checks whether the vertex shader is expecting an input attribute by a
    // given name, such as "vVertex" or "vNormal."
    bool HasAttribute(std::string const &name) const;

    // Retrieves the index (OpenGL intrinsic value) associated to a uniform
    // constant within the shader program, given its name.
    u32 GetUniform(std::string const &name);

    // Retrieves the index (OpenGL intrinsic value) associated to a vertex
    // attribute within the shader program, given its name.
    u32 GetAttribute(std::string const &name);

    // Uploads the vertex and fragment shader code to the GPU, creates shader
    // objects for those shaders, and compiles them. Creates the shader program
    // object, attaches the compiled vertex and fragment shaders to the
    // program and links them. It finishes with validating the program and
    // cleaning up the shader objects used to construct it.
    void Build();

    // Binds this shader program for uses including setting uniform constant
    // values and using the program to render geometry.
    void Bind() const;

    // Sets a uniform Vector4, given a name. This will send all 4 floats of the
    // Vector4 to the GPU.
    void SetUniform(std::string const &name, Math::Vector4 const &vector);

	void SetUniform(std::string const &name, Math::Vector3 const &vector);

    // Sets a uniform Matrix4, given a name. This will send all 16 floats of the
    // Matrix4 to the GPU.
    void SetUniform(std::string const &name, Math::Matrix4 const &matrix);

    // Sets a uniform Color, given a name. Colors are equivalent to vec4s in
    // GLSL, therefore this just sends all 4 floats of the RGBA color (in that
    // order), to the GPU.
    void SetUniform(std::string const &name, Color const &color);

    // Sets a uniform 32-bit, single precision float value, given a name. This
    // sends the single float to the GPU.
    void SetUniform(std::string const &name, f32 value);

    // Sets a uniform 32-bit integral value, given a name. This sends the
    // integer value to the GPU. The sign of this data type depends on how it
    // is used within GLSL.
    void SetUniform(std::string const &name, u32 value);

    // Unbinds the shader program, disallowing it to be used for any future
    // OpenGL operations until it is bound again.
    void Unbind() const;

    // Loads a new ShaderProgram from disk, given relative paths to its vertex
    // and fragment shader source files. These files are expected to be relative
    // to assets/shaders, from the root directory. The function loads in these
    // files in completion, then constructs a new ShaderProgram with the
    // contents of those shaders. It does not call ShaderProgram::Build.
    static std::shared_ptr<ShaderProgram> LoadShaderProgram(
      std::string const &vertexShaderPath,
      std::string const &fragmentShaderPath);

  private:
    ShaderProgram(std::string const &vertexShaderSource,
      std::string const &fragmentShaderSource,
      std::string const &vertexShaderPath,
      std::string const &fragmentShaderPath);

    u32 program_; /* OpenGL handle to the ShaderProgram object. */

    std::string vertexShaderPath_, fragmentShaderPath_;

    /* Actual GLSL source code for the program's vertex and fragment shaders. */
    std::string vertexShaderSource_, fragmentShaderSource_;

    /* Cached index maps of the uniforms/attributes that were set and get. */
    std::unordered_map<std::string, u32> uniforms_, attributes_;
  };
}

#endif
