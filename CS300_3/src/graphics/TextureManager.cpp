#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "graphics/TextureManager.h"

namespace
{
	// Kept as a file-scope global so that it can be returned as a const ref.
	static std::shared_ptr<Graphics::Texture> const NullTexture = NULL;
}

namespace Graphics
{
	TextureManager::TextureManager() : textures_()
	{
		std::memset(slotBindings_, false, sizeof(slotBindings_));
	}

	TextureManager::~TextureManager()
	{
		ClearTextures();
	}

	std::shared_ptr<Texture> const &TextureManager::RegisterTexture(
		TextureType type, std::string const &textureFilePath)
	{
		// load the texture given the specified image file; save the texture given
		// the specified type or, if one is already associated with that type,
		// replace it with the newly loaded texture

		auto texture = Texture::LoadTGA(textureFilePath);
		auto find = textures_.find(type);
		if (find != textures_.end())
		{
			find->second = texture; // replace texture
			return find->second;    // and return it
		}

		// new texture: insert(std::pair<TextureType, ...>(type, texture))
		return textures_.emplace(type, texture).first->second;
	}

	std::shared_ptr<Texture> const &TextureManager::RegisterNormalMapTexture(TextureType type,
	  std::string const &heightmapFilePath)
	{
		auto texture = Texture::LoadNormalMapFromHeightMapTGA(
			heightmapFilePath, 
			DiscreteDifferentialMethod::Central, 
			TextureWrapType::ClampToZero,
			TextureWrapType::ClampToZero
			);
		auto find = textures_.find(type);
		if (find != textures_.end())
		{
			find->second = texture; // replace texture
			return find->second;    // and return it
		}

		// new texture: insert(std::pair<TextureType, ...>(type, texture))
		return textures_.emplace(type, texture).first->second;
	}

	std::shared_ptr<Texture> const &TextureManager::GetTexture(TextureType type) const
	{
		// find a texture given the specified type; if it doesn't exist, return
		// null instead
		auto find = textures_.find(type);
		return (find != textures_.end()) ? find->second : NullTexture;
	}

	bool TextureManager::BindAttach(TextureType type,
		std::shared_ptr<ShaderProgram> const &program,
		std::string const &samplerUniformName)
	{
		auto &texture = GetTexture(type);
		if (!texture)
		{
			WarnIf(true, "Warning: cannot bind/attach with no texture registered to"
				" type: %d", type);
			return false; // no such texture exists
		}

		// find slot to bind to
		u8 slot = findNextOpenSlot();
		if (slot == -1)
			return false; // no slot to bind to

		// We COULD optimize this process by making sure textures are not rebound
		// continuously; ie, the same texture should remember it's slot. It's good
		// practice to unbind textures, though in a high-performing engine you want
		// to minimize binds and context switches as often as possible. This
		// framework errs on the side of illustrating good practice over efficiency.
		texture->Bind(slot);
		texture->AttachTexture(program, samplerUniformName);
		slotBindings_[slot] = true; // texture bound at this slot

		return true; // texture exists and it was bound successfully
	}

	bool TextureManager::Unbind(TextureType type)
	{
		auto &texture = GetTexture(type);
		if (!texture)
		{
			WarnIf(true, "Warning: cannot unbind with no texture registered to"
				" type: %d", type);
			return false; // no such texture exists
		}

		if (texture->IsBound())
			slotBindings_[texture->boundSlot_] = false; // texture is now unbound

		// try to unbind (cannot be bound already)
		texture->Unbind();

		return true;
	}

	void TextureManager::ClearTextures()
	{
		// free up resources consumed by registered textures
		textures_.clear();
	}

	u8 TextureManager::findNextOpenSlot() const
	{
		// attempt to find an open slot for a texture to be bound
		for (u8 i = 0; i < NumberAvailableTextureUnits; ++i)
			if (!slotBindings_[i])
				return i;
		Assert(false, "Warning: failed to find an open slot in texture manager."
			" All texture slots consumed. Did you forget to unbind your textures?");
		return -1;
	}
}
