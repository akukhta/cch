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
            std::vector<char> compress(std::span<char> data);
        };
    }
}
