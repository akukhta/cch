#pragma once
#include <string>
#include <algorithm>

namespace cch::hashing
{
    struct djb2
    {
        static size_t djb2hash(size_t prevHash, char c)
        {
            return ((prevHash << 5) + prevHash) + c;
        }

        static size_t djb2hash(std::string const& str)
        {
            size_t hash = defaultHashValue;

            std::ranges::for_each(str, [&hash](char c) { hash = cch::hashing::djb2::djb2hash(hash, c); });
            return hash;
        }

        static size_t const inline defaultHashValue = 5381;
    };
}