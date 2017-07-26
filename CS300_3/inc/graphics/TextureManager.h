#ifndef H_TEXTURE_MANAGER
#define H_TEXTURE_MANAGER

#include "framework/Utilities.h"

namespace Graphics
{
  class ShaderProgram;
  class Texture;

  // Similar in purpose to ShaderType found in ShaderManager.
  enum class TextureType
  {
	DIFFUSE,
	SPECULAR,
	NORMAL,
    COUNT
  };

  // This is the number of slots available in this particular instance of
  // OpenGL for multitexturing. Ie, it's the total number of textures that can
  // simultaneously be bound and access at a time in a shader. This number is
  // usually 16 or 32, but it is being fixed at 8 for the purposes of CS300
  // assignments.w
  static u8 const NumberAvailableTextureUnits = 8;

  // Similar in purpose to the ShaderManager. Handling multiple textures at
  // once in OpenGL is its own beast. This manager helps provide an easy way to
  // deal with multiple textures being bound at once.
  class TextureManager
  {
  public:

    TextureManager();
    ~TextureManager(); // delete all registered textures (and unbind them)

    // Registers a new texture given a path to an image file (either PNG or
    // TGA), as well as the type it should be bound to. If a texture has
    // already been loaded to a specific type (such as EXAMPLE), then the
    // existing texture will be replaced with the new texture loaded from the
    // file.
	std::shared_ptr<Texture> const &RegisterTexture(TextureType type,
	  std::string const &textureFilePath);

	std::shared_ptr<Texture> const &RegisterNormalMapTexture(TextureType type,
	  std::string const &heightmapFilePath);

    // Retreives a texture based on its TextureType. If the type is unknown,
    // this method returns NULL.
    std::shared_ptr<Texture> const &GetTexture(TextureType key) const;
    // Binds a texture stored within this manager given the specified type. It
    // will also attach the texture to a shader program given the uniform name
    // of the sampler the texture should be bound to. This should be the only
    // method needed to prepare shader programs to use a particular texture.
    // Furthermore, make sure you are also calling Unbind after you are done
    // rendering with a particular texture.
    bool BindAttach(TextureType type,
      std::shared_ptr<ShaderProgram> const &program,
      std::string const &samplerUniformName);

    // Unbinds a texture registered inside this manager given its type. This
    // method returns false if no such texture exists within this manager.
    bool Unbind(TextureType type);

    // Destroys all textures stored within this manager. This is handy to
    // cleanup any texture resources being used by the application.
    void ClearTextures();

  private:
    u8 findNextOpenSlot() const;

    // Disallow copying of this object.
    TextureManager(TextureManager const &) = delete;
    TextureManager &operator=(TextureManager const &) = delete;

    std::map<TextureType, std::shared_ptr<Texture>> textures_;
    bool slotBindings_[NumberAvailableTextureUnits];
  };
}

#endif
