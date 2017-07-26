#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/TriangleMesh.h"
#include "graphics/Vertex.h"
#include "graphics/Texture.h"
#include "math/Math.h"
#include "math/Reals.h"

namespace Graphics
{
	using namespace Math;

	TriangleMesh::Triangle::Triangle(u32 _a, u32 _b, u32 _c)
		: a(_a), b(_b), c(_c)
	{
	}

	TriangleMesh::TriangleMesh()
		: vertices_()
		, triangles_()
		, triangleNormals_()
		, vertexArrayObject_(nullptr)
		, vaoVertexNormals_(nullptr)
		, vaoVertexTangents_(nullptr)
		, vaoVertexBitangents_(nullptr)
		, vaoFaceNormals_(nullptr)
		, textureMappingType_(TextureProjectorFunction::CYLINDRICAL)
	{
	}

	TriangleMesh::~TriangleMesh()
	{
	}

	void TriangleMesh::AddVertex(f32 x, f32 y, f32 z)
	{
		vertices_.emplace_back(Vector3(x, y, z));
	}

	void TriangleMesh::AddTriangle(u32 a, u32 b, u32 c)
	{
		triangles_.emplace_back(a, b, c);
	}

	//////////////////////////////////////////////////////////////////////////
	// various useful steps for preparing this model for rendering; none of
	// these would be done for a game

	// TODO(student): implement computing face and vertex normals; remember,
	// you do not want adjacent faces with the same normal to affect subsequent
	// vertices that are shared by those two faces (otherwise it will bias the
	// vertex normal's direction toward those two faces, which is incorrect)
	//////////////////////////////////////////////////////////////////////////

	struct VectorLessThan {
		bool operator()(const Vector3& a, const Vector3& b)
		{
			return a.x < b.x ||
				a.x == b.x && a.y < b.y ||
				a.x == b.x && a.y == b.y && a.z < b.z;
		}
	};

	void TriangleMesh::generateUV()
	{
		for (unsigned i = 0; i < vertices_.size(); ++i)
		{
			Vertex& v = vertices_[i];
			switch (textureMappingType_)
			{
			case TextureProjectorFunction::CYLINDRICAL:
			{
				v.uv.x = Math::Clamp(atan2f(v.vertex.z, v.vertex.x) / Math::cTwoPi, -1.f, 1.f);
				if (v.uv.x < 0)
					v.uv.x += 1;
				v.uv.y = Math::Clamp(v.vertex.y + 0.5f, 0.f, 1.f);
			} break;

			case TextureProjectorFunction::SPHERICAL:
			{
				v.uv.x = Math::Clamp(atan2f(v.vertex.z, v.vertex.x) / Math::cTwoPi, -1.f, 1.f);
				if (v.uv.x < 0)
					v.uv.x += 1;
				float r = v.vertex.Length();
				v.uv.y = Math::Clamp(acosf(v.vertex.y / r), 0.f, Math::cPi) / Math::cPi;
			} break;

			//case TextureProjectorFunction::CUBIC:
			//{
			//	Assert(false, "CUBIC not supported yet!");
			//} break;
			}
		}
	}

	void TriangleMesh::generateTBN()
	{
		// this is where I generate TBN vectors
		triangleNormals_.reserve(triangles_.size());
		triangleTangents_.reserve(triangles_.size());
		triangleBytangents_.reserve(triangles_.size());
		std::vector<std::set<Vector3, VectorLessThan> > vertexNormalSets(vertices_.size());
		std::vector<std::set<Vector3, VectorLessThan> > vertexTangentSets(vertices_.size());
		std::vector<std::set<Vector3, VectorLessThan> > vertexBitangentSets(vertices_.size());
		for (auto &t : triangles_)
		{
			// the formula is correct I think
			Vector3 v1 = vertices_[t.b].vertex - vertices_[t.a].vertex;
			Vector3 v2 = vertices_[t.c].vertex - vertices_[t.a].vertex;

			Vector2 t1 = vertices_[t.b].uv - vertices_[t.a].uv;
			Vector2 t2 = vertices_[t.c].uv - vertices_[t.a].uv;

			Vector3 normal = v1.Cross(v2).Normalized();
			Vector3 tan = t2.y * v1 - t1.y * v2;
			Vector3 bitan = t2.x * v1 - t1.x * v2;
			f32 tanNormalizer = t1.x * t2.y - t1.y * t2.x;
			if (tanNormalizer == 0.f)
				tanNormalizer = FLT_EPSILON;

			tan /= tanNormalizer;
			if (tan.LengthSq() > Math::Sq(100))
				tan.Normalize();

			bitan /= -tanNormalizer;
			if (bitan.LengthSq() > Math::Sq(100))
				bitan.Normalize();

			triangleNormals_.emplace_back(normal);
			triangleTangents_.emplace_back(tan);
			triangleBytangents_.emplace_back(bitan);

			f32 c1 = normal.Dot(tan);
			f32 c2 = normal.Dot(bitan);
			f32 c3 = bitan.Dot(tan);

			for (auto i = 0; i < 3; ++i)
			{
				vertexNormalSets[t.indices[i]].insert(normal);
				vertexTangentSets[t.indices[i]].insert(tan);
				vertexBitangentSets[t.indices[i]].insert(bitan);
			}
		}

		for (unsigned i = 0; i < vertices_.size(); ++i)
		{
			if (!vertexNormalSets[i].empty())
			{
				auto v = Vector3(0, 0, 0);
				for (auto &n : vertexNormalSets[i])
					v += n;
				v /= static_cast<f32>(vertexNormalSets[i].size());
				v.AttemptNormalize();
				vertices_[i].normal = v;
			}

			if (!vertexTangentSets[i].empty())
			{
				auto v = Vector3(0, 0, 0);
				for (auto &t : vertexTangentSets[i])
					v += t;
				v /= static_cast<f32>(vertexTangentSets[i].size());
				vertices_[i].tangent = v;
			}

			if (!vertexBitangentSets[i].empty())
			{
				auto v = Vector3(0, 0, 0);
				for (auto &b : vertexBitangentSets[i])
					v += b;
				v /= static_cast<f32>(vertexBitangentSets[i].size());
				vertices_[i].bitangent = v;
			}


			f32 c1 = vertices_[i].normal.Dot(vertices_[i].tangent);
			f32 c2 = vertices_[i].normal.Dot(vertices_[i].bitangent);
			f32 c3 = vertices_[i].bitangent.Dot(vertices_[i].tangent);
			Assert(true, "%f, %f, %f", c1, c2, c3);
		}
	}

	void TriangleMesh::Preprocess()
	{
		centerMesh();
		normalizeVertices();

		generateUV();
		generateTBN();
	}

	int TriangleMesh::GetVertexCount() const
	{
		return vertices_.size();
	}

	int TriangleMesh::GetTriangleCount() const
	{
		return triangles_.size();
	}

	Math::Vector3 TriangleMesh::GetTriangleCentroid(u32 tidx) const
	{
		// average all vertices of the triangle to find the centroid
		Triangle const &tri = triangles_[tidx];
		Vector3 centroid = vertices_[tri.a].vertex;
		centroid += vertices_[tri.b].vertex;
		centroid += vertices_[tri.c].vertex;
		return centroid * (1.f / 3.f);
	}

	Vertex const &TriangleMesh::GetVertex(u32 vidx) const
	{
		Assert(vidx < vertices_.size(), "Error: vertex index out of"
			" bounds: %d", vidx);
		return vertices_[vidx];
	}

	Vertex &TriangleMesh::GetVertex(u32 vidx)
	{
		Assert(vidx < vertices_.size(), "Error: vertex index out of"
			" bounds: %d", vidx);
		return vertices_[vidx];
	}

	TriangleMesh::Triangle const &TriangleMesh::GetTriangle(u32 tidx) const
	{
		Assert(tidx < triangles_.size(), "Error: triangle index out of"
			" bounds: %d", tidx);
		return triangles_[tidx];
	}

	u32 TriangleMesh::GetPolygonIndex(u32 tidx, u8 vertex) const
	{
		Assert(tidx < triangles_.size(), "Error: triangle index out of"
			" bounds: %d", tidx);
		Assert(vertex < 3, "Error: vertex index within triangle out of bounds: %d"
			" (expected 0 <= vertex < 3)", vertex);
		return triangles_[tidx].indices[vertex];
	}

	Vector3 const &TriangleMesh::GetPolygonNormal(u32 tidx) const
	{
		Assert(tidx < triangleNormals_.size(), "Error: triangle normal index"
			" out of bounds: %d", tidx);
		return triangleNormals_[tidx];
	}

	void TriangleMesh::Build(std::shared_ptr<ShaderProgram> program)
	{
		// Construct a new VAO using each of the triangles and vertices stored
		// within this TriangleMesh; this is done in a less efficient way possible.
		// A simple memcpy could actually be used to copy data from the mesh
		// directly into the IBO and VBO.
		vertexArrayObject_ = std::shared_ptr<VertexArrayObject>(new VertexArrayObject(vertices_.size(), triangles_.size()));
		auto &vbo = vertexArrayObject_->GetVertexBufferObject();
		auto &ibo = vertexArrayObject_->GetIndexBufferObject();
		vaoVertexNormals_ = std::shared_ptr<VertexArrayObject>(new VertexArrayObject(vertices_.size() * 2, vertices_.size(), Topology::LINES));
		auto &VNvbo = vaoVertexNormals_->GetVertexBufferObject();
		auto &VNibo = vaoVertexNormals_->GetIndexBufferObject();
		vaoVertexTangents_ = std::shared_ptr<VertexArrayObject>(new VertexArrayObject(vertices_.size() * 2, vertices_.size(), Topology::LINES));
		auto &VTvbo = vaoVertexTangents_->GetVertexBufferObject();
		auto &VTibo = vaoVertexTangents_->GetIndexBufferObject();
		vaoVertexBitangents_ = std::shared_ptr<VertexArrayObject>(new VertexArrayObject(vertices_.size() * 2, vertices_.size(), Topology::LINES));
		auto &VBvbo = vaoVertexBitangents_->GetVertexBufferObject();
		auto &VBibo = vaoVertexBitangents_->GetIndexBufferObject();
		vaoFaceNormals_ = std::shared_ptr<VertexArrayObject>(new VertexArrayObject(triangles_.size() * 2, triangles_.size(), Topology::LINES));
		auto &FNvbo = vaoFaceNormals_->GetVertexBufferObject();
		auto &FNibo = vaoFaceNormals_->GetIndexBufferObject();

		for (size_t i = 0; i < vertices_.size(); ++i)
		{
			vbo.AddVertex(vertices_[i]);

			VNvbo.AddVertex(Vertex(vertices_[i].vertex));
			VNvbo.AddVertex(Vertex(vertices_[i].vertex + vertices_[i].normal * 0.1f));
			VNibo.AddLine(i * 2, i * 2 + 1);

			VTvbo.AddVertex(Vertex(vertices_[i].vertex));
			VTvbo.AddVertex(Vertex(vertices_[i].vertex + vertices_[i].tangent * 0.1f));
			VTibo.AddLine(i * 2, i * 2 + 1);

			VBvbo.AddVertex(Vertex(vertices_[i].vertex));
			VBvbo.AddVertex(Vertex(vertices_[i].vertex + vertices_[i].bitangent * 0.1f));
			VBibo.AddLine(i * 2, i * 2 + 1);
		}

		for (size_t i = 0; i < triangles_.size(); ++i)
		{
			Triangle &tri = triangles_[i];
			ibo.AddTriangle(tri.a, tri.b, tri.c);

			FNvbo.AddVertex(Vertex((vertices_[tri.a].vertex + vertices_[tri.b].vertex + vertices_[tri.c].vertex) / 3.f));
			FNvbo.AddVertex(Vertex(((vertices_[tri.a].vertex + vertices_[tri.b].vertex + vertices_[tri.c].vertex) / 3.f) + triangleNormals_[i] * 0.1f));

			FNibo.AddLine(i * 2, i * 2 + 1);
		}

		// upload the contents of the VBO and IBO to the GPU and build the VAO
		vertexArrayObject_->Build(program);
		vaoVertexNormals_->Build(program);
		vaoVertexTangents_->Build(program);
		vaoVertexBitangents_->Build(program);
		vaoFaceNormals_->Build(program);
	}

	void TriangleMesh::Render()
	{
		// if the VAO has been built for this mesh, bind and render it
		if (vertexArrayObject_)
		{
			vertexArrayObject_->Bind();
			vertexArrayObject_->Render();
			vertexArrayObject_->Unbind();
		}
	}
	
	void TriangleMesh::RenderVertexNormals()
	{
		if (vaoVertexNormals_)
		{
			vaoVertexNormals_->Bind();
			vaoVertexNormals_->Render();
			vaoVertexNormals_->Unbind();
		}
	}

	void TriangleMesh::RenderVertexTangents()
	{
		if (vaoVertexTangents_)
		{
			vaoVertexTangents_->Bind();
			vaoVertexTangents_->Render();
			vaoVertexTangents_->Unbind();
		}
	}

	void TriangleMesh::RenderVertexBitangents()
	{
		if (vaoVertexBitangents_)
		{
			vaoVertexBitangents_->Bind();
			vaoVertexBitangents_->Render();
			vaoVertexBitangents_->Unbind();
		}
	}

	void TriangleMesh::RenderFaceNormals()
	{
		if (vaoFaceNormals_)
		{
			vaoFaceNormals_->Bind();
			vaoFaceNormals_->Render();
			vaoFaceNormals_->Unbind();
		}
	}

	/* helper methods */

	void TriangleMesh::centerMesh()
	{
		// find the centroid of the entire mesh (average of all vertices, hoping for
		// no overflow) and translate all vertices by the negative of this centroid
		// to ensure all transformations are about the origin
		Vector3 centroid(0.f);
		for (auto &vert : vertices_)
			centroid += vert.vertex;
		centroid *= 1.f / static_cast<f32>(vertices_.size());
		// translate by negative centroid to center model at (0, 0, 0)
		centroid = -centroid;
		for (auto &vert : vertices_)
			vert.vertex += centroid;
	}

	void TriangleMesh::normalizeVertices()
	{
		// find the extent of this mesh and normalize all vertices by scaling them
		// by the inverse of the smallest value of the extent
		minimum_ = vertices_[0].vertex;
		maximum_ = vertices_[0].vertex;
		for (auto &vert : vertices_)
		{
			f32 x = vert.vertex.x, y = vert.vertex.y, z = vert.vertex.z;
			minimum_.x = std::min(minimum_.x, x);
			minimum_.y = std::min(minimum_.y, y);
			minimum_.z = std::min(minimum_.z, z);
			maximum_.x = std::max(maximum_.x, x);
			maximum_.y = std::max(maximum_.y, y);
			maximum_.z = std::max(maximum_.z, z);
		}
		Vector3 extent = maximum_ - minimum_;
		f32 scaler = 1;
		if (extent.x > 0 && scaler > extent.x)
			scaler = extent.x;
		if (extent.y > 0 && scaler > extent.y)
			scaler = extent.y;
		if (extent.z > 0 && scaler > extent.z)
			scaler = extent.z;

		scaler = 1 / scaler;
		for (auto &vert : vertices_)
		{
			vert.vertex *= scaler;
		}
		maximum_ = maximum_ * scaler;
		minimum_ = minimum_ * scaler;
	}

}
