#include "mtl_parser.h"

std::string _space;

std::string parse_map_path(std::string_view cmdArgs) {
	std::istringstream argStream(cmdArgs.data());
	std::string path;
	argStream >> path;
	return path;
}

float parse_float_value(std::string_view cmdArgs) {
	std::istringstream argStream(cmdArgs.data());
	std::string value;
	argStream >> value;
	return std::stof(value);
}

DirectX::XMFLOAT2 parse_float2_value(std::string_view cmdArgs) {
	std::istringstream argStream(cmdArgs.data());
	std::string x, y;
	argStream >> x >> _space >> y;
	return { std::stof(x),std::stof(y) };
}

DirectX::XMFLOAT3 parse_float3_value(std::string_view cmdArgs) {
	std::istringstream argStream(cmdArgs.data());
	std::string x, y, z;
	argStream >> x >> _space >> y >> _space >> z;
	return { std::stof(x),std::stof(y),std::stof(z) };
}

void parse_map_scale(material_file& mtl_file, std::string_view cmdArgs) {
	std::istringstream argStream(cmdArgs.data());
	std::string u, v;
	argStream >> u >> _space >> v;
	mtl_file.scale = { std::stof(u), (std::stof(v)) };
}

void parse_diffuse_color(material_file& mtl_file, std::string_view cmdArgs) {
	auto rgb = parse_float3_value(cmdArgs);
	mtl_file.values.albedo = { rgb.x,rgb.y,rgb.z,1.0f };
}

void parse_diffuse_alpha(material_file& mtl_file, std::string_view cmdArgs) {
	mtl_file.values.albedo.w = parse_float_value(cmdArgs);
}

void parse_normal_strength(material_file& mtl_file, std::string_view cmdArgs) {
	std::istringstream argStream(cmdArgs.data());
	std::string multiCmd;

	argStream >> multiCmd;
	if (multiCmd == "-bm") {
		std::string bmValue;
		argStream >> multiCmd >> bmValue;
		mtl_file.values.normal = std::stof(bmValue);
		return;
	}
	mtl_file.values.normal = 1.0f;
}

void parse_diffuse_map(material_file& mtl_file, std::string_view cmdArgs) {
	mtl_file.paths.albedo = parse_map_path(cmdArgs);
}

void parse_normal_map(material_file& mtl_file, std::string_view cmdArgs) {
	std::istringstream argStream(cmdArgs.data());
	std::string multiCmd;

	argStream >> multiCmd;
	if (multiCmd == "-bm") {
		std::string bmValue;
		argStream >> multiCmd >> bmValue >> multiCmd >> multiCmd;
		mtl_file.paths.normal = multiCmd;
	}
	else {
		mtl_file.paths.normal = multiCmd;
	}
}

void parse_roughness_map(material_file& mtl_file, std::string_view cmdArgs) {
	mtl_file.paths.roughness = parse_map_path(cmdArgs);
}

void parse_roughness_value(material_file& mtl_file, std::string_view cmdArgs) {
	float Ns = parse_float_value(cmdArgs);
	Ns = 1.0f - sqrt(Ns / 1000.0f);
	mtl_file.values.roughness = Ns;
}

void parse_metallic_map(material_file& mtl_file, std::string_view cmdArgs) {
	mtl_file.paths.metallic = parse_map_path(cmdArgs);
}


std::unordered_map<std::string, std::function<void(material_file& mtl_file, std::string_view)>> parseFunction{
	{"Kd", parse_diffuse_color },
	{"map_Kd",parse_diffuse_map},
	{"map_Bump",parse_normal_map},
	{"map_Ns",parse_roughness_map},
	{"Ns",parse_roughness_value},
	{"map_refl",parse_metallic_map}
};

std::vector<material_file> load_mtl_material(std::string path, process_status& status) {

	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		status = failed_fileopen;
	}

	int mtlCount = 0;
	std::vector<material_file> materials;

	material_file* mtl = nullptr;

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == ' ') continue;

		std::istringstream sstream(line);
		std::string cmd;

		sstream >> cmd;
		std::string args = line.substr(cmd.size() + 1);
		if (cmd == "newmtl") {
			materials.push_back({});
			mtl = &materials.back();
			mtl->name = args;
			mtlCount++;
			continue;
		}
		else {
			
			try{
				std::string_view _args(args);
				const auto& function = parseFunction[cmd];
				function(*mtl, _args);
			}
			catch (std::exception& e) {
				continue;
			}
		}
	}

	file.close();
	return materials;
}
