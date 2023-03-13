#pragma once

#include "Slick.h"

namespace Slick::Editor {

	struct GLTFPrimitive {
		u32 material;
		u32 indices;

		i32 attribute_position;
		i32 attribute_normal;
		i32 attribute_tangent;
		i32 attribute_texcoord;
	};

	struct GLTFMesh {
		std::string name;
		std::vector<GLTFPrimitive> primitives;
	};

	enum struct GLTFElementType {
		FLOAT,
		UNSIGNED_SHORT
	};

	enum struct GLTFType {
		VEC4,
		VEC3,
		VEC2,
		SCALAR
	};

	struct GLTFAccessor {
		u32 buffer_view;
		u32 count;
		GLTFElementType component_type;
		GLTFType type;
		Math::fVec3 min, max;
	};

	struct GLTFBufferView {
		u32 buffer;
		u32 length;
		u32 offset;
	};
	
	struct GLTFBuffer {
		std::vector<u8> data;
	};

	struct GLTFMaterial {
		std::string name;
		Math::fVec3 base_color;
		i32 base_color_texture;

		float metallic, roughness;
		i32 metallic_roughness_texture;

		i32 normal_texture;
	};

	enum struct ImageFormat {
		PNG,
	};

	struct GLTFImage {
		ImageFormat format;
		std::string name;
		u32 buffer_view;
	};

	struct GLTFTexture {
		u32 source;
		u32 sampler;
	};

	struct GLTFScene {
		// Mesh data
		std::vector<GLTFMesh> meshes;
		std::vector<GLTFAccessor> accessors;
		std::vector<GLTFBufferView> buffer_views;
		std::vector<GLTFBuffer> buffers;
		// Material data
		std::vector<GLTFImage> images;
		std::vector<GLTFTexture> textures;
		std::vector<GLTFMaterial> materials;
	};

	GLTFScene load_gltf(const std::string& fname);

}