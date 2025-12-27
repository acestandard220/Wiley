#include "uuid.h"

namespace Wiley {
    UUID UUIDFactory::invalid{ 0,0 };
    UUID UUIDFactory::generateUUID()
    {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis(0, 0xFFFFFFFFFFFFFFFF);

        UUID uuid;
        uuid.high = dis(gen);
        uuid.low = dis(gen);

        uuid.high &= 0xFFFFFFFFFFFF0FFFULL;
        uuid.high |= 0x0000000000004000ULL;

        uuid.low &= 0x3FFFFFFFFFFFFFFFULL;
        uuid.low |= 0x8000000000000000ULL;

        return uuid;
    }

    std::string UUIDFactory::uuidToString(const UUID& uuid)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        uint64_t h = uuid.high;
        uint64_t l = uuid.low;

        ss << std::setw(8) << ((h >> 32) & 0xFFFFFFFF) << "-";
        ss << std::setw(4) << ((h >> 16) & 0xFFFF) << "-";
        ss << std::setw(4) << (h & 0xFFFF) << "-";
        ss << std::setw(4) << ((l >> 48) & 0xFFFF) << "-";
        ss << std::setw(12) << (l & 0xFFFFFFFFFFFFULL);

        return ss.str();
    }

    std::string removeHyphens(const std::string& s)
    {
        std::string result;
        std::copy_if(s.begin(), s.end(), std::back_inserter(result),
            [](char c) { return c != '-'; });
        return result;
    }

    uint64_t hexToUint64(const std::string& hex)
    {
        uint64_t value = 0;
        std::stringstream ss;
        ss << std::hex << hex;
        ss >> value;
        return value;
    }

    UUID UUIDFactory::uuidFromString(const std::string& s)
    {
        std::string hex = removeHyphens(s);
        if (hex.size() != 32)
        {
            throw std::runtime_error("Invalid UUID string");
        }

        UUID uuid;
        uuid.high = hexToUint64(hex.substr(0, 16));
        uuid.low = hexToUint64(hex.substr(16, 16));
        return uuid;
    }
}