#include "GLTFLoader.h"

#include "nlohmann/json.hpp"

#include <string>

namespace Slick::Editor {

	using namespace nlohmann;

	GLTFPrimitive parse_primitive(json& j) {
		GLTFPrimitive prim{};

		prim.indices = j["indices"];
		prim.material = j["material"];

		prim.attribute_position = j["attributes"]["POSITION"];
		prim.attribute_normal = j["attributes"].contains("NORMAL") ? (i32)j["attributes"]["NORMAL"] : -1;
		prim.attribute_tangent = j["attributes"].contains("TANGENT") ? (i32)j["attributes"]["TANGENT"] : -1;
		prim.attribute_texcoord = j["attributes"].contains("TEXCOORD_0") ? (i32)j["attributes"]["TEXCOORD_0"] : -1;

		return prim;
	};
	std::vector<u8> parse_data_uri(json& j) {
		std::string data = j;

		enum struct State {
			DataPrefix,
			MediaType,
			Base64Specifier,
			Data
		} state = State::DataPrefix;

		size_t offset = 0;
		auto peek = [&]() { return data[offset]; };

		bool isBase64 = false;
		std::string mediaType;
		std::vector<u8> result;
		while (offset < data.size()) {
			switch (state) {
			case State::DataPrefix:
			{
				if (peek() == ':') {
					state = State::MediaType;
					offset += 1;
				}
				else {
					offset += 1;
				}
				continue;
			}
			case State::MediaType:
			{
				if (peek() == ';') {
					state = State::Base64Specifier;
					offset += 1;
				}
				else if (peek() == ',') {
					state = State::Data;
					offset += 1;
					result.reserve(offset * 3 / 4);
				}
				else {
					mediaType.push_back(data[offset++]);
				}
				continue;
			}
			case State::Base64Specifier:
			{
				isBase64 = true;
				if (peek() == ',') {
					state = State::Data;
					result.reserve(offset * 3 / 4);
				}
				offset++;
				continue;
			}
			case State::Data:
			{
				u8 lut[256];
				for (char c = 'A'; c <= 'Z'; c++) {
					lut[c] = c - 'A' + 0;
				}
				for (char c = 'a'; c <= 'z'; c++) {
					lut[c] = c - 'a' + 26;
				}
				for (char c = '0'; c <= '9'; c++) {
					lut[c] = c - '0' + 52;
				}
				lut['+'] = 62;
				lut['/'] = 63;

				intptr_t left = data.size() - offset;
				if (left >= 4) {
					u8 a = lut[data[offset + 0]];
					u8 b = lut[data[offset + 1]];
					u8 c = lut[data[offset + 2]];
					u8 d = lut[data[offset + 3]];

					u8 x = (a << 2) | (b >> 4);
					u8 y = (b << 4) | (c >> 2);
					u8 z = (c << 6) | d;

					result.push_back(x);
					result.push_back(y);
					result.push_back(z);

					offset += 4;
				}
				else {
					Utility::Assert(false);
				}
				continue;
			}
			}
		}

		return result;
	};


	GLTFScene load_gltf(const std::string& fname) {
		Utility::ScopeTimer st("load_gltf");

		std::fstream fs(fname, std::fstream::in);
		if (!fs) {
			Utility::Log("Failed to open file. ", fname);
			return {};
		}
		std::string file;
		fs.seekg(0, fs.end);
		file.resize(fs.tellg());
		fs.seekg(0, fs.beg);
		fs.read(file.data(), file.size());

		json gltf;
		{
			Utility::ScopeTimer pt("json parse");
			gltf = json::parse(file);
		}

		GLTFScene scene{};

		for (auto& mesh : gltf["meshes"]) {
			GLTFMesh rmesh{};
			rmesh.name = mesh["name"];
			for (auto& primitive : mesh["primitives"]) {
				rmesh.primitives.push_back(parse_primitive(primitive));
			}
			scene.meshes.push_back(rmesh);
		}

		for (auto& acc : gltf["accessors"]) {
			GLTFAccessor accessor{};

			accessor.buffer_view = acc["bufferView"];
			accessor.count = acc["count"];

			if (acc.contains("max")) {
				if (acc["max"].size() == 3)
					accessor.max = { acc["max"][0], acc["max"][1], acc["max"][2] };
				else if (acc["max"].size() == 2)
					accessor.max = { acc["max"][0], acc["max"][1], 0.f };
				else if (acc["max"].size() == 1)
					accessor.max = { acc["max"][0], 0.f, 0.f };
				else
					Utility::Assert(false);
			}
			else
				accessor.max = { 0.f, 0.f, 0.f };

			if (acc.contains("min")) {
				if (acc["min"].size() == 3)
					accessor.min = { acc["min"][0], acc["min"][1], acc["min"][2] };
				else if (acc["min"].size() == 2)
					accessor.min = { acc["min"][0], acc["min"][1], 0.f };
				else if (acc["min"].size() == 1)
					accessor.min = { acc["min"][0], 0.f, 0.f };
				else
					Utility::Assert(false);
			}
			else
				accessor.max = { 0.f, 0.f, 0.f };

			u32 cType = acc["componentType"];
			if (cType == 5126)           accessor.component_type = GLTFElementType::FLOAT;
			else if (cType == 5123)           accessor.component_type = GLTFElementType::UNSIGNED_SHORT;
			else Utility::Assert(false);

			std::string type = acc["type"];
			if (type == "VEC4")          accessor.type = GLTFType::VEC4;
			else if (type == "VEC3")     accessor.type = GLTFType::VEC3;
			else if (type == "VEC2")     accessor.type = GLTFType::VEC2;
			else if (type == "SCALAR")   accessor.type = GLTFType::SCALAR;
			else Utility::Assert(false);

			scene.accessors.push_back(accessor);
		}

		for (auto& bufView : gltf["bufferViews"]) {
			GLTFBufferView bv{};

			bv.buffer = bufView["buffer"];
			bv.length = bufView["byteLength"];
			bv.offset = bufView["byteOffset"];

			scene.buffer_views.push_back(bv);
		}

		for (auto& buf : gltf["buffers"]) {
			GLTFBuffer b{};

			b.data = parse_data_uri(buf["uri"]);

			scene.buffers.emplace_back(std::move(b));
		}

		for (auto& mat : gltf["materials"]) {
			GLTFMaterial m{};

			if (mat["pbrMetallicRoughness"].contains("baseColorFactor")) {
				m.base_color = Math::fVec3{
					mat["pbrMetallicRoughness"]["baseColorFactor"][0],
					mat["pbrMetallicRoughness"]["baseColorFactor"][1],
					mat["pbrMetallicRoughness"]["baseColorFactor"][2]
				};
			}
			else {
				m.base_color = { 0.5f, 0.5f, 0.5f };
			}

			if (mat["pbrMetallicRoughness"].contains("baseColorTexture")) {
				m.base_color_texture = mat["pbrMetallicRoughness"]["baseColorTexture"]["index"];
			}
			else {
				m.base_color_texture = -1;
			}

			if (mat["pbrMetallicRoughness"].contains("metallicRoughnessTexture")) {
				m.metallic_roughness_texture = mat["pbrMetallicRoughness"]["metallicRoughnessTexture"]["index"];
				m.metallic = 0.5f;
				m.roughness = 0.5f;
			}
			else {
				m.metallic_roughness_texture = -1;
			}

			m.normal_texture = mat.contains("normalTexture") ? (i32)mat["normalTexture"]["index"] : -1;

			m.metallic = mat["pbrMetallicRoughness"].contains("metallicFactor")
				? (float)mat["pbrMetallicRoughness"]["metallicFactor"]
				: 0.5f;
			m.roughness = mat["pbrMetallicRoughness"].contains("roughnessFactor")
				? (float)mat["pbrMetallicRoughness"]["roughnessFactor"]
				: 0.5f;

			scene.materials.push_back(m);
		}

		for (auto& img : gltf["images"]) {
			GLTFImage i{};

			i.buffer_view = img["bufferView"];
			i.name = img["name"];

			std::string mimeType = img["mimeType"];
			if (mimeType == "image/png") i.format = ImageFormat::PNG;
			else Utility::Assert(false);

			scene.images.push_back(i);
		}

		for (auto& img : gltf["textures"]) {
			GLTFTexture i{};

			i.sampler = img["sampler"];
			i.source = img["source"];

			scene.textures.push_back(i);
		}

		return scene;
	}

}

