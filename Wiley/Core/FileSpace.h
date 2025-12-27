#pragma once
#include <filesystem>
#include <iostream>

namespace Wiley
{

	namespace filespace {
		using filepath = std::filesystem::path;

		static bool Exists(filepath path) {
			return std::filesystem::exists(path);
		}

		static std::string Extension(filepath path) {
			if (std::filesystem::is_directory(path)) {
				std::cout << "Path provided is a directory. Cannot get the extension of a directory." << std::endl;
				return "";
			}
			return path.extension().string();
		}
	}

}
