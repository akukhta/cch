#pragma once
#include <span>
#include <vector>
#include "../config/types.h"

namespace cch
{
    namespace compression
    {
        class RLECompression
        {
        public:
            std::vector<cch::byte> compress(std::span<cch::byte> data);
            std::vector<cch::byte> decompress(std::span<cch::byte> data);
        };
    }
}
