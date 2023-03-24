#include "ResourceManager.h"

#include "glad/glad.h"

#include "graphics/mesh/GLTFLoader.h"
#include "stb_image.h"

namespace Slick::App {

	Format format_from_ctype_and_type(Loader::GLTFElementType ct, Loader::GLTFType t) {
		if (ct == Loader::GLTFElementType::FLOAT) {
			if (t == Loader::GLTFType::VEC4) {
				return Format::Float4;
			}
			else if (t == Loader::GLTFType::VEC3) {
				return Format::Float3;
			}
			else if (t == Loader::GLTFType::VEC2) {
				return Format::Float2;
			}
		}
		else if (ct == Loader::GLTFElementType::UNSIGNED_SHORT) {
			if (t == Loader::GLTFType::SCALAR) {
				return Format::UInt16;
			}
		}
		Utility::Assert(false);
		return Format::Unknown;
	};

	ResourceManager::ResourceManager() {}

	ResourceManager::~ResourceManager() {}

	std::vector<std::pair<u32, u32>> ResourceManager::generate_meshes_from_gltf(const Loader::GLTFScene& gltf) {
		std::vector<u32> buffer_ids;
		for (auto& buffer : gltf.buffers) {
			u32 id{};
			glGenBuffers(1, &id);
			glBindBuffer(GL_ARRAY_BUFFER, id);
			glBufferData(GL_ARRAY_BUFFER, buffer.data.size(), buffer.data.data(), GL_STATIC_DRAW);
			buffer_ids.push_back(id);
		}

		std::vector<u32> texture_ids;
		for (auto& img : gltf.images) {
			auto& bv = gltf.buffer_views[img.buffer_view];
			auto& b = gltf.buffers[bv.buffer];

			i32 w{}, h{}, c{};
			u8* img_data = stbi_load_from_memory(b.data.data() + bv.offset, bv.length, &w, &h, &c, 4);

			u32 texId{};
			glGenTextures(1, &texId);
			glBindTexture(GL_TEXTURE_2D, texId);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)img_data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			stbi_image_free(img_data);

			texture_ids.push_back(texId);
		}

		std::vector<std::pair<u32, u32>> result;

		for (auto& mesh : gltf.meshes) {
			for (auto& prim : mesh.primitives) {
				auto& mat = gltf.materials[prim.material];

				auto get_material = [&]() {
					Material material{
						.baseColor = mat.base_color,
						.baseColorTexture = -1,
						.metallic = mat.metallic,
						.roughness = mat.roughness,
						.metallicRoughnessTexture = -1,
						.normalTexture = -1,
					};

					auto get_texture = [&](i32 texture) -> i32 {
						if (texture != -1) {
							auto& tex = gltf.textures[texture];
							u32 texid = texture_ids[tex.source];
							return texid;
						}
						return -1;
					};

					material.baseColorTexture = get_texture(mat.base_color_texture);
					material.metallicRoughnessTexture = get_texture(mat.metallic_roughness_texture);
					material.normalTexture = get_texture(mat.normal_texture);

					mMaterials.push_back(material);

					return mMaterials.size() - 1;
				};

				auto get_mesh = [&]() {
					Mesh rc{};
					auto get_buffer = [&](i32 attribute_accessor) -> std::tuple<BufferReference, bool, u32> {
						if (attribute_accessor < 0)
							return { {}, false, 0 };

						auto& accessor = gltf.accessors[attribute_accessor];
						auto& bufferView = gltf.buffer_views[accessor.buffer_view];

						return {
							BufferReference{
								buffer_ids[bufferView.buffer],
								bufferView.offset,
								format_from_ctype_and_type(accessor.component_type, accessor.type)
							},
							true,
							accessor.count
						};
					};

					auto [pBuf, pOk, pCount] = get_buffer(prim.attribute_position);
					rc.position = pBuf;

					auto [nBuf, nOk, nCount] = get_buffer(prim.attribute_normal);
					rc.normal = nBuf;
					rc.hasNormals = nOk;

					auto [tBuf, tOk, tCount] = get_buffer(prim.attribute_tangent);
					rc.tangent = tBuf;
					rc.hasTangents = tOk;

					auto [uBuf, uOk, uCount] = get_buffer(prim.attribute_texcoord);
					rc.uvs = uBuf;
					rc.hasUvs = uOk;

					auto [iBuf, iOk, iCount] = get_buffer(prim.indices);
					rc.indices = iBuf;
					rc.hasIndices = iOk;
					rc.drawCount = iCount;

					mMeshes.push_back(rc);

					return mMeshes.size() - 1;
				};

				auto material = get_material();
				auto mesh = get_mesh();

				result.push_back({mesh, material});
			}
		}

		return result;
	}

}
