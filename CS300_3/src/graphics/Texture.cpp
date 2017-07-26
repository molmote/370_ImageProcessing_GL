#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "math/Vector3.h"

#include <STB/stb_image.h>

static u8 const UnbuiltTexture = 0;
static u8 const UnboundTexture = -1;

// Converts the relative path to a useful relative path (based on the local
// directory of the executable). This assumes ASSET_PATH is correct.
static std::string GetFilePath(std::string const &relativePath)
{
	std::stringstream strstr;
	strstr << ASSET_PATH << "textures/" << relativePath;
	return strstr.str();
}

namespace Graphics
{
	Texture::Texture(u8 *pixels, u32 width, u32 height)
	  : pixels_(pixels), width_(width), height_(height),
	  textureHandle_(UnbuiltTexture), boundSlot_(UnboundTexture)
	{
	}

	Texture::~Texture()
	{
		Destroy();
		delete[] pixels_;
	}

	void Texture::Build()
	{
		Assert(textureHandle_ == UnbuiltTexture,
		  "Cannot build already built texture.");

		// create a new texture
		glGenTextures(1, &textureHandle_);

		// bind the generated texture and upload its image contents to OpenGL
		glBindTexture(GL_TEXTURE_2D, textureHandle_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB,
		  GL_UNSIGNED_BYTE, pixels_);

		// unbind the texture
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::Bind(u8 slot)
	{
		// Enable the texture unit given the specified slot, then bind this texture
		// as the 2D texture for that slot.

		WarnIf(textureHandle_ == UnbuiltTexture,
		  "Warning: Cannot bind unbuilt texture.");
		WarnIf(boundSlot_ != UnboundTexture,
		  "Warning: Cannot rebind texture until it is unbound.");
		glActiveTexture(GL_TEXTURE0 + slot); // bind texture to slot
		glBindTexture(GL_TEXTURE_2D, textureHandle_);
		boundSlot_ = slot;
	}

	bool Texture::IsBound() const
	{
		return boundSlot_ != UnboundTexture;
	}

	void Texture::AttachTexture(std::shared_ptr<ShaderProgram> const &shader,
	  std::string const &uniformName)
	{
		// bind the texture slot to the uniform since this is the activated texture
		// in that slot (allows the shader to sample the sampler given the input
		// name, wherein it will directly be pulling pixel data from this texture
		// instance).
		shader->SetUniform(uniformName, u32(static_cast<u32>(boundSlot_)));
	}

	void Texture::Unbind()
	{
		WarnIf(boundSlot_ == UnboundTexture,
		  "Warning: Cannot unbind unbound texture.");
		glActiveTexture(GL_TEXTURE0 + boundSlot_); // unbind texture from slot
		glBindTexture(GL_TEXTURE_2D, 0);
		boundSlot_ = UnboundTexture; // unbound
	}

	void Texture::Destroy()
	{
		if (textureHandle_ == UnbuiltTexture)
		  return; // nothing to destroy
		if (boundSlot_ != UnboundTexture)
		  Unbind();
		glDeleteTextures(1, &textureHandle_); // wipe out the texture
		textureHandle_ = UnbuiltTexture;
	}

	u32 Texture::GetWidth() const
	{
		return width_;
	}

	u32 Texture::GetHeight() const
	{
		return height_;
	}

	std::shared_ptr<Texture> Texture::LoadTGA(std::string const &path)
	{
		// STB image handles the type based on the format itself
		return LoadPNG(path);
	}

	std::shared_ptr<Texture> Texture::LoadPNG(std::string const &relative)
	{
		// convert relative path (to textures) to a fully qualified relative path
		// (relative to the executable itself)
		std::string path = GetFilePath(relative);

		// attempt to load a PNG/TGA file using STB Image
		int width = 0, height = 0, bpp = 0;
		void *data = stbi_load(path.c_str(), &width, &height, &bpp, STBI_rgb);

		Assert(data, "Error: Unable to read file: textures/%s, reason: %s",
		  relative.c_str(), stbi_failure_reason());
		if (!data)
		  return NULL; // failed to load

		Assert(bpp == 3, "Error: Can only handle RGB images in the CS300 framework."
		  " No alpha channels supported. Read file with bpp=%d", bpp);

		Texture *texture = NULL;
		if (bpp == 3) // successfully read an image with 3 channels of data
		{
			// copy data from loaded STB image buffer to local pixel buffer
			u8 *pixelData = new u8[width * height * bpp];
			std::memcpy(pixelData, data, width * height * bpp);
			texture = new Texture(pixelData, u32(width), u32(height));
		}

		stbi_image_free(data);
		return std::shared_ptr<Texture>(texture);
	}

	static f32 GetHeight(const u8* pixelData, int i, int j, int width, int height, int bpp, TextureWrapType wrapType)
	{
		const int rowSize = width * bpp;
		if (i >= 0 && i < height && j >= 0 && j < width)
			return pixelData[i * rowSize + j * bpp] / 255.f;

		if (wrapType == TextureWrapType::ClampToZero)
			return 0.f;

		if (wrapType == TextureWrapType::RepeatBoundary)
		{
			int newi = Math::Clamp(i, 0, height - 1);
			int newj = Math::Clamp(j, 0, width - 1);
			return pixelData[newi * rowSize + newj * bpp] / 255.f;
		}

		Assert(false, "wrapType %d Not supported", (int)wrapType);
		return 0.f;
	}

	static std::pair<f32, f32> Differentiate(int i, int j, const u8* pixelData, int width, int height, int bpp, TextureWrapType hwrapType, TextureWrapType vwrapType)
	{
		f32 scaler = 5.f;
		f32 hfwd = GetHeight(pixelData, i + 1, j, width, height, bpp, hwrapType);
		f32 hbwd = GetHeight(pixelData, i - 1, j, width, height, bpp, hwrapType);
		f32 hz = scaler * (hfwd - hbwd);
		hfwd = GetHeight(pixelData, i, j + 1, width, height, bpp, vwrapType);
		hbwd = GetHeight(pixelData, i, j - 1, width, height, bpp, vwrapType);
		f32 vz = scaler * (hfwd - hbwd);
		return std::pair<f32, f32>(hz, vz);
	}


	std::shared_ptr<Texture> Texture::LoadNormalMapFromHeightMapTGA(
		std::string const &relative, 
		DiscreteDifferentialMethod diffMethod, 
		TextureWrapType vwType, 
		TextureWrapType hwType)
	{
		// convert relative path (to textures) to a fully qualified relative path
		// (relative to the executable itself)
		std::string path = GetFilePath(relative);

		// attempt to load a PNG/TGA file using STB Image
		int width = 0, height = 0, bpp = 0;
		void *data = stbi_load(path.c_str(), &width, &height, &bpp, STBI_rgb);

		Assert(data, "Error: Unable to read file: textures/%s, reason: %s",
		  relative.c_str(), stbi_failure_reason());
		if (!data)
		  return nullptr; // failed to load

		Texture* texture = nullptr;
		Assert(bpp == 3, "Error: Can only handle RGB images in the CS300 framework."
		  " No alpha channels supported. Read file with bpp=%d", bpp);
		if (bpp == 3) // successfully read an image with 3 channels of data
		{
			// copy data from loaded STB image buffer to local pixel buffer
			u8 *pixelData = new u8[width * height * bpp];
			u8 *normalData = new u8[width * height * 3];
			std::memcpy(pixelData, data, width * height * bpp);
			int const rowSize = width * bpp;
			for(int i = 0; i < height; ++i)
			{
				for(int j = 0; j < width; ++j)
				{
					std::pair<float, float> heights = Differentiate(i, j, 
						pixelData, width, height, bpp,
						hwType, vwType);

					Math::Vector3 s = Math::Vector3(1, 0, heights.first);
					Math::Vector3 t = Math::Vector3(0, 1, heights.second);
					Math::Vector3 n = s.Cross(t).Normalized();
					Math::Vector3 colored = (n + Math::Vector3(1, 1, 1)) * 0.5f * 255.f;
					normalData[i * rowSize + j * bpp + 0] = (u8)colored.x;
					normalData[i * rowSize + j * bpp + 1] = (u8)colored.y;
					normalData[i * rowSize + j * bpp + 2] = (u8)colored.z;
				}
			}
			texture = new Texture(normalData, u32(width), u32(height));
			delete[] pixelData;
		}
		stbi_image_free(data);
		return std::shared_ptr<Texture>(texture);
	}
}
