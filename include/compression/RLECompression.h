#pragma once
#include <span>
#include <vector>

namespace cch
{
    namespace compression
    {
        class RLECompression
        {
        public:
            std::vector<unsigned char> compress(std::span<unsigned char> data);
            std::vector<unsigned char> decompress(std::span<unsigned char> data);
        };
    }
}
