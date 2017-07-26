#include "Precompiled.h"
#include "framework/Debug.h"
#include "framework/Utilities.h"
#include "graphics/Color.h"
#include "graphics/ShaderProgram.h"
#include "math/Matrix4.h"
#include "math/Vector4.h"
#include "math/Vector3.h"

namespace
{
	// Converts the relative path to a useful relative path (based on the local
	// directory of the executable). This assumes ASSET_PATH is correct.
	static std::string GetFilePath(std::string const &relativePath)
	{
		std::stringstream strstr;
		strstr << ASSET_PATH << "shaders/" << relativePath;
		return strstr.str();
	}

	// Performs a very inefficient (but conveniently small) way of reading the
	// entire shader text file as a string.
	static std::string ReadFile(std::string const &relativePath)
	{
		std::string file = GetFilePath(relativePath);
		std::ifstream fstream = std::ifstream(file);
		Assert(fstream.good(), "Failed to read file: %s", file.c_str());

		// convenient (but slow) way of reading an entire file into a string
		return std::string(std::istreambuf_iterator<char>(fstream),
			std::istreambuf_iterator<char>());
	}

	// Verifies that the shader has compiled successfully. If anything went wrong,
	// it will warn with the message provided by the graphics driver. Each
	// shader object (and including the program object) contains an info log based
	// on compile and link operations. This is a very handy method to keep around
	// when you want to receive error messages for compiler errors in your
	// shaders. Call this after calling glCompileShader.
	static bool VerifyShaderCompilation(std::string const &filePath, u32 shader)
	{
		GLint status = GL_TRUE; // assume it succeeded
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status); // query if it did

		if (status == GL_FALSE) // did it actually?
		{
			GLint infoLogLength;
			std::string infoLog;

			// get the number of characters stored inside the shader's info log
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
			infoLog.reserve(infoLogLength); // prime string to contain the log

			// retrieve the info log and save inside a std::string
			glGetShaderInfoLog(shader, infoLogLength, NULL, &infoLog[0]);

			// warn that something went wrong
			WarnIf(true, "Error compiling shader \"%s\":\n%s",
				filePath.c_str(), infoLog.c_str());

			return false;
		}

		return true;
	}

	// Similar to VerifyShaderCompilation, except it makes sure the entire shader
	// program successfully linked together all shader objects attached to it.
	// ShaderProgram::Build explains the concept of linking in greater detail.
	// Call this after calling glLinkProgram.
	static bool VerifyProgramLinking(std::string const &vsFilePath,
		std::string const &fsFilePath, u32 program)
	{
		GLint status = GL_TRUE; // assume it succeeded
		glGetProgramiv(program, GL_LINK_STATUS, &status); // query if it did

		if (status == GL_FALSE) // did it actually?
		{
			GLint infoLogLength;
			std::string infoLog;

			// query the number of characters in the program's info log
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
			infoLog.reserve(infoLogLength); // prime the string to contain the log

			// retrieve the info log and save inside a std::string
			glGetProgramInfoLog(program, infoLogLength, NULL, &infoLog[0]);

			// warn that something went wrong
			WarnIf(true, "Error linking program %d (contains vertex shader \"%s\""
				" and fragment shader \"%s\"): %s", program, vsFilePath.c_str(),
				fsFilePath.c_str(), infoLog.c_str());

			return false;
		}

		return true;
	}

	// Similar to VerifyProgramLinking. Operates on ShaderPrograms. Call this
	// after calling glValidateProgram.
	static bool VerifyProgramValidation(std::string const &vsFilePath,
		std::string const &fsFilePath, u32 program)
	{
		GLint status = GL_TRUE; // assume success
		glGetProgramiv(program, GL_VALIDATE_STATUS, &status); // query if it did

		if (status == GL_FALSE) // did it actually?
		{
			GLint infoLogLength;
			std::string infoLog;

			// query the number of characters in the program's info log
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
			infoLog.reserve(infoLogLength); // prime the string to contain the log

			// retrieve the info log and save inside a std::string
			glGetProgramInfoLog(program, infoLogLength, NULL, &infoLog[0]);

			// warn that something went wrong
			WarnIf(true, "Error validating program %d (contains vertex shader \"%s\""
				" and fragment shader \"%s\"): %s", program, vsFilePath.c_str(),
				fsFilePath.c_str(), infoLog.c_str());

			return false;
		}

		return true;
	}
}

namespace Graphics
{
	ShaderProgram::ShaderProgram(std::string const &vertexShaderSource,
		std::string const &fragmentShaderSource,
		std::string const &vertexShaderPath,
		std::string const &fragmentShaderPath)
		: program_(0), vertexShaderSource_(vertexShaderSource),
		fragmentShaderSource_(fragmentShaderSource),
		vertexShaderPath_(vertexShaderPath),
		fragmentShaderPath_(fragmentShaderPath)
	{
	}

	ShaderProgram::~ShaderProgram()
	{
		// cleanup
		glDeleteProgram(program_);
	}

	bool ShaderProgram::HasUniform(std::string const &name) const
	{
		Assert(program_ != 0, "Cannot get uniform from unbuilt shader: %s",
			name.c_str());
		// try to find the uniform in a map before calling glGetUniformLocation;
		// this avoids calls to glGetUniformLocation which is considered slow
		auto find = uniforms_.find(name);
		if (find != uniforms_.end())
			return true;
		return glGetUniformLocation(program_, name.c_str()) != -1;
	}

	bool ShaderProgram::HasAttribute(std::string const &name) const
	{
		Assert(program_ != 0, "Cannot get attribute from unbuilt shader: %s",
			name.c_str());
		// try to find the attribute in a map before calling glGetAttribLocation;
		// this avoids calls to glGetAttribLocation which is considered slow
		auto find = attributes_.find(name);
		if (find != attributes_.end())
			return true;
		return glGetAttribLocation(program_, name.c_str()) != -1;
	}

	u32 ShaderProgram::GetUniform(std::string const &name)
	{
		Assert(program_ != 0, "Cannot get uniform from unbuilt shader: %s",
			name.c_str());

		// First tries to find the uniform in the uniforms_ map; if it cannot be
		// found, it then gets the location from OpenGL and saves it in the map to
		// avoid ever needing to call glGetUniformLocation again.
		auto find = uniforms_.find(name);
		u32 location;
		if (find == uniforms_.end())
		{
			s32 slocation = glGetUniformLocation(program_, name.c_str());
			WarnIf(slocation == -1, "No uniform in program by name: %s", name.c_str());
			location = static_cast<u32>(slocation);
			uniforms_.insert(std::make_pair(name, location));
		}
		else
			location = find->second;
		return location;
	}

	u32 ShaderProgram::GetAttribute(std::string const &name)
	{
		Assert(program_ != 0, "Cannot get attribute from unbuilt shader: %s",
			name.c_str());

		// First tries to find the uniform in the attributes_ map; if it cannot be
		// found, it then gets the location from OpenGL and saves it in the map to
		// avoid ever needing to call glGetAttribLocation again.
		auto find = attributes_.find(name);
		u32 location;
		if (find == attributes_.end())
		{
			s32 sloc = glGetAttribLocation(program_, name.c_str());
			Assert(sloc != -1, "No attribute in program by name: %s", name.c_str());
			location = static_cast<u32>(sloc);
			attributes_.insert(std::make_pair(name, location));
		}
		else
			location = find->second;
		return location;
	}

	void ShaderProgram::Build()
	{
		// creates a new shader program; programs represent linked and compiled
		// shaders and are used to actually render geometry
		u32 program = glCreateProgram();

		// create a shader object for both vertex and fragment shaders
		u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
		u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		Assert(program != 0, "Error creating shader program.");
		Assert(vertexShader != 0, "Error creating vertex shader.");
		Assert(fragmentShader != 0, "Error creating fragment shader.");

		char const *vshader = vertexShaderSource_.c_str();
		GLint vshaderLength = vertexShaderSource_.length();
		char const *fshader = fragmentShaderSource_.c_str();
		GLint fshaderLength = fragmentShaderSource_.length();

		// upload the shader source code to the graphics driver
		glShaderSource(vertexShader, 1, &vshader, &vshaderLength);
		glShaderSource(fragmentShader, 1, &fshader, &fshaderLength);

		// compile and verify the vertex shader
		glCompileShader(vertexShader);
		VerifyShaderCompilation(vertexShaderPath_, vertexShader);

		// compile and verify the fragment shader
		glCompileShader(fragmentShader);
		VerifyShaderCompilation(fragmentShaderPath_, fragmentShader);

		// attach the compiled shaders to the program
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		// link the program together so that it can be used to render
		glLinkProgram(program);
		VerifyProgramLinking(vertexShaderPath_, fragmentShaderPath_, program);

		// validate the program is operational in the current OpenGL state
		glValidateProgram(program);
		VerifyProgramValidation(vertexShaderPath_, fragmentShaderPath_, program);

		// Per the OpenGL specification, it is not required to hold on to these
		// shader objects for the lifetime of the program. Therefore, we clean up
		// their resources and only hold onto the compiled, linked, and complete
		// program.
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		program_ = program;
	}

	void ShaderProgram::Bind() const
	{
		Assert(program_ != 0, "Cannot bind unbuilt shader.");

		// indicate to OpenGL we want to use this program to render geometry or
		// set uniforms
		glUseProgram(program_);

		// ensure we can use this program in the current OpenGL context
#ifdef _DEBUG
		glValidateProgram(program_);
		VerifyProgramValidation(vertexShaderPath_, fragmentShaderPath_, program_);
#endif
	}

	void ShaderProgram::SetUniform(std::string const &name,
		Math::Vector4 const &vector)
	{
		// glUniform4fv sets a vec4 using an array of floats
		u32 location = GetUniform(name);
		glUniform4fv(location, 1, vector.ToFloats());
	}

	void ShaderProgram::SetUniform(std::string const &name,
		Math::Vector3 const &vector)
	{
		// glUniform4fv sets a vec4 using an array of floats
		u32 location = GetUniform(name);
		glUniform3fv(location, 1, vector.ToFloats());
	}

	void ShaderProgram::SetUniform(std::string const &name,
		Math::Matrix4 const &matrix)
	{
		// glUniformMatrix4f sets a 4x4 matrix using an array of floats; GL_TRUE
		// indicates to transpose the matrix upon upload to the GPU
		u32 location = GetUniform(name);
		glUniformMatrix4fv(location, 1, GL_TRUE, matrix.array);
	}

	void ShaderProgram::SetUniform(std::string const &name, Color const &color)
	{
		// uploads the color to a vec4 using an array of floatss
		u32 location = GetUniform(name);
		glUniform4fv(location, 1, color.ToFloats());
	}

	void ShaderProgram::SetUniform(std::string const &name, f32 value)
	{
		// uploads the raw float value to the GPU
		u32 location = GetUniform(name);
		glUniform1f(location, value);
	}

	void ShaderProgram::SetUniform(std::string const &name, u32 value)
	{
		// uploads the raw integer value to the GPU
		u32 location = GetUniform(name);
		glUniform1i(location, value);
	}

	void ShaderProgram::Unbind() const
	{
		// indicates to the driver not to use any program to render currently by
		// using the special reserved program handle 0
		glUseProgram(NULL);
	}

	std::shared_ptr<ShaderProgram> ShaderProgram::LoadShaderProgram(
		std::string const &vertexShaderPath,
		std::string const &fragmentShaderPath)
	{
		// construct a new shader program by reading in the vertex and fragment
		// shader files
		return std::shared_ptr<ShaderProgram>(new ShaderProgram(
			ReadFile(vertexShaderPath), ReadFile(fragmentShaderPath),
			vertexShaderPath, fragmentShaderPath));
	}
}
