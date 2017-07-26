#ifndef H_TEXTURE
#define H_TEXTURE

#include "framework/Utilities.h"

namespace Graphics
{
  class ShaderProgram;

  enum class DiscreteDifferentialMethod
  {
	  Central,
	  Forward,
	  Backward,
	  Count
  };

  enum class TextureWrapType
  {
	  RepeatBoundary,
	  ClampToZero,
	  Count
  };


  enum class TextureProjectorFunction
  {
	  CYLINDRICAL = 0,
	  SPHERICAL,
	  COUNT
  };

  // By far, one of the best explanations I've come across that explains how
  // the concept of multitexturing works in OpenGL (the accepted answer):
  //   http://stackoverflow.com/questions/8866904
  // This class represents an OpenGL texture, loaded from a PNG or TGA file.
  // Textures may be bound, attached to a uniform in a shader (sampler type),
  // or unbound. They are similar in concept to other types of OpenGL objects,
  // though interacting with them on the shader is different, given they have a
  // much different purpose to serve.
  class Texture
  {
  public:
    ~Texture();

    // builds the texture and uploads it to the graphics card
    void Build();
    void Bind(u8 slot); // bind texture to specific texture slot
    bool IsBound() const;
    void AttachTexture(std::shared_ptr<ShaderProgram> const &shader,
      std::string const &uniformName);
    void Unbind();
    void Destroy();
    u32 GetWidth() const;
    u32 GetHeight() const;

    static std::shared_ptr<Texture> LoadTGA(std::string const &path);
    static std::shared_ptr<Texture> LoadPNG(std::string const &path);
	static std::shared_ptr<Texture> LoadNormalMapFromHeightMapTGA(
		std::string const &relative,
		DiscreteDifferentialMethod diffMethod,
		TextureWrapType vtype,
		TextureWrapType htype);

    friend class TextureManager;
  private:
    Texture(u8 *pix, u32 width, u32 height);

    u8 *pixels_;
    u32 width_, height_;
    u32 textureHandle_;
    u8 boundSlot_;
  };
}

#endif
