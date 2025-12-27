#pragma once
#include <stdint.h>
#include <string>
#include <memory>
#include <random>
#include <sstream>
#include <string_view>
#include <functional>
#include <filesystem>

#define WILEY_GEN_UUID UUIDFactory::generateUUID()
#define WILEY_INVALID_UUID UUIDFactory::InvalidUUID()
#define WILEY_UUID_STRING(uuid) UUIDFactory::uuidToString(uuid)
#define WILEY_UUID_FROM_STRING(uuid) UUIDFactory::uuidFromString(uuid)

namespace Wiley {
	struct UUID
	{
		uint64_t high;
		uint64_t low;

		bool operator==(const UUID& other) const
		{
			return high == other.high && low == other.low;
		}

		bool operator!=(const UUID& other) const
		{
			return !(*this == other);
		}

		bool operator<(const UUID& other) const
		{
			return (high < other.high) || (high == other.high && low < other.low);
		}
	};

	class UUIDFactory
	{
		public:
			static UUID generateUUID();
			static UUID InvalidUUID() { return invalid; }

			static std::string uuidToString(const UUID& uuid);
			static UUID uuidFromString(const std::string& s);
		private:
			static UUID invalid;
	};
}
namespace std
{
	template<>
	struct hash<Wiley::UUID>
	{
		std::size_t operator()(const Wiley::UUID& uuid) const noexcept
		{
			return std::hash<uint64_t>{}(uuid.high) ^ (std::hash<uint64_t>{}(uuid.low) << 1);
		}
	};
}
