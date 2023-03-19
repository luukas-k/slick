#pragma once

#include "Core.h"

#include "math/Vec.h"

namespace Slick::App {

	enum struct Format : u8 {
		Unknown,

		Float4,
		Float3,
		Float2,
		Float1,
		UInt16,
	};

	struct BufferReference {
		u32 buffer, offset;
		Format fmt;
	};

	struct Mesh {
		/*i32 posBuffer, posOffset;
		Format posFormat;*/
		BufferReference position;
		
		BufferReference normal;
		bool hasNormals;

		BufferReference tangent;
		bool hasTangents;

		BufferReference uvs;
		bool hasUvs;

		BufferReference indices;
		bool hasIndices;

		u32 drawCount;
	};

	struct Material {
		Math::fVec3 baseColor;
		i32 baseColorTexture;

		float metallic, roughness;
		i32 metallicRoughnessTexture;

		i32 normalTexture;
	};

	class ResourceManager {
	public:
		ResourceManager();
		~ResourceManager();

		std::vector<std::pair<u32, u32>> load_mesh(const std::string& fname);
		inline const Mesh& get_mesh_by_id(u32 id){ return mMeshes[id]; }
		inline const Material& get_material_by_id(u32 id) { return mMaterials[id]; }
	private:
		std::vector<Material> mMaterials;
		std::vector<Mesh> mMeshes;
	};

}