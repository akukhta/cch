#pragma once
#include <string>
#include <algorithm>

namespace cch::hash
{
    /// Hash algorithm for strings
    struct djb2
    {
        /// Non constructible
        djb2() = delete;

        /// Update a hash for a new char added to the string with O(1) complexity
        /// \param prevHash previous hash value
        /// \param c new char added to the string
        /// \return updated hash value
        static size_t djb2hash(size_t prevHash, char c)
        {
            return ((prevHash << 5) + prevHash) + c;
        }

        /// Calculate a hash for a string with O(n) complexity
        /// \param str input string
        /// \return hash value
        static size_t djb2hash(std::string const& str)
        {
            size_t hash = defaultHashValue;

            std::ranges::for_each(str, [&hash](char c) { hash = cch::hash::djb2::djb2hash(hash, c); });
            return hash;
        }

        /// Default hash value aka magic constant
        static size_t const inline defaultHashValue = 5381;
    };
}