#include "Precompiled.h"
#include "graphics/ShaderManager.h"
#include "graphics/ShaderProgram.h"

namespace
{
  // Kept as a file-scope global so that it can be returned as a const ref.
  static std::shared_ptr<Graphics::ShaderProgram> const NullProgram = NULL;
}

namespace Graphics
{
  ShaderManager::ShaderManager()
    : shaders_()
  {
  }

  ShaderManager::~ShaderManager()
  {
  }

  std::shared_ptr<ShaderProgram> const &ShaderManager::RegisterShader(
    ShaderType type, std::string const &vertexSourceFile,
    std::string const &fragmentSourceFile)
  {
    // load the shader program given the specified source files; save the shader
    // given the specified type or, if one is already associated with that type,
    // replace it with the newly loaded shader program
    auto find = shaders_.find(type);
    auto program = ShaderProgram::LoadShaderProgram(vertexSourceFile,
      fragmentSourceFile);
    if (find != shaders_.end())
    {
      find->second = program; // replace program
      return find->second;    // and return it
    }
    else // new shader: insert(std::pair<ShaderType, ...>(type, program))
      return shaders_.emplace(type, program).first->second; // and return it
  }

  std::shared_ptr<ShaderProgram> const &ShaderManager::GetShader(
    ShaderType type) const
  {
    // find a shader given the specified type; if it doesn't exist, return null
    // instead
    auto find = shaders_.find(type);
    return (find != shaders_.end()) ? find->second : NullProgram;
  }

  void ShaderManager::ClearShaders()
  {
    shaders_.clear(); // delete all shader program instances registered
  }
}
