#include "Precompiled.h"
#include "framework/Debug.h"
#include "framework/Utilities.h"
#include "graphics/MeshLoader.h"
#include "graphics/TriangleMesh.h"

namespace Graphics
{
	//////////////////////////////////////////////////////////////////////////
	// TODO(student): implement a Wavefront OBJ loader in this file, assemble
	// an instance of TriangleMesh using the data read in, and return it

	// NOTE: the 'input' stream above is ready to being reading the text lines
	// of OBJ files; it already is setup to correctly find the object file
	// under the assets/models directory.
	//////////////////////////////////////////////////////////////////////////
	std::shared_ptr<TriangleMesh> MeshLoader::LoadMesh(std::string const &objFile, TextureProjectorFunction textureMappingType)
	{
		std::stringstream strstr;
		strstr << ASSET_PATH << "models/" << objFile;
		std::ifstream input = std::ifstream(strstr.str());
		Assert(input.good(), "Cannot load mesh: assets/models/%s", objFile.c_str());
		if (!input.good())
		  return nullptr;

		TriangleMesh *mesh = new TriangleMesh;
		std::string line;
		std::string flag;
		while (std::getline(input, line))
		{
			if(line.length() == 0)
				continue;

			switch (line[0])
			{
			case 'v':
			{
				float x, y, z;
				std::istringstream iss(line);
				iss >> flag >> x >> y >> z;
				mesh->AddVertex(x, y, z);
				break;
			}
			case 'f':
			{
				int v[3];
				std::istringstream iss(line);
				iss >> flag >> v[0] >> v[1] >> v[2];
				mesh->AddTriangle(v[0]-1, v[1]-1, v[2]-1);
				break;
			}
			default:
			{
				break;
			}
			}
		}

		mesh->SetTextureMappingType(textureMappingType);
		mesh->Preprocess();
		return std::shared_ptr<TriangleMesh>(mesh);
	}
}