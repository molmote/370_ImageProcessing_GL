#ifndef H_SHADER_MANAGER
#define H_SHADER_MANAGER

namespace Graphics
{
  class ShaderProgram;

  // This enum stores all the known types of shaders used by the application.
  // Add more to this enum as you create more shader programs.
  enum class ShaderType
  {
    PHONG_LIGHT = 0,
	PHONG_SHADE,
	BLINN_SHADE,
	DEBUG,
	BLINN_NORMAL,
	PHONG_NORMAL,
	PHONGLIGHT_NORMAL,
    // TODO(student): add more shader types here, such as PHONG or GOURAUD

    COUNT
  };

  // This class is responsible for storing all shader programs loaded within
  // the framework. For more information on what a shader program is or how
  // they work, please refer to ShaderProgram.h. This class associates a shader
  // program with a type (as defined in the ShaderType enum above), simplifying
  // shader usage throughout the application. This allows shaders to be easily
  // reloaded without changing any places where the shader is being used (see
  // ShaderManager::RegisterShader for how shader reloading works).
  class ShaderManager
  {
  public:
    ShaderManager();
    ~ShaderManager();

    // Registers a new shader given its source vertex and fragment shader files,
    // as well as the type it should be bound to. If a shader has already been
    // loaded to a specific type (such as EXAMPLE), then the existing
    // shader will be replaced with the new shader loaded from the file.
    std::shared_ptr<ShaderProgram> const &RegisterShader(ShaderType type,
      std::string const &vertexSourceFile,
      std::string const &fragmentSourceFile);

    // Retreives a shader program based on its ShaderType. If the type is
    // unknown, this method returns NULL.
    std::shared_ptr<ShaderProgram> const &GetShader(ShaderType key) const;

    // Destroys all shader programs stored within this manager. This is handy
    // to cleanup any shader resources being used by the application.
    void ClearShaders();

  private:
    // Disallow copying of this object.
    ShaderManager(ShaderManager const &) = delete;
    ShaderManager &operator=(ShaderManager const &) = delete;

    std::unordered_map<ShaderType, std::shared_ptr<ShaderProgram>> shaders_;
  };
}

#endif
