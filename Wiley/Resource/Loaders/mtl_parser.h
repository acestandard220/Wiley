#pragma once
#include <DirectXMath.h>

#include <string_view>
#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <functional>

struct material_file {
	struct {
		DirectX::XMFLOAT4 albedo;
		float normal;
		float ao;
		float roughness;
		float metallic;
	}values;

	struct {
		std::string albedo;
		std::string normal;
		std::string ao;
		std::string roughness;
		std::string metallic;
	}paths;
	std::string name;
	DirectX::XMFLOAT2 scale;
};
 
enum process_status {
	successful,
	successful_incom,
	failed_fileopen
};

std::vector<material_file> load_mtl_material(std::string path,process_status& status);
