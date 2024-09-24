#pragma once
#include <span>
#include <sstream>
#include <iomanip>
#include <vector>
#include <bit>
#include <array>
#include "config/types.h"

namespace cch::hash
{
    struct SHA256Hash
    {
        union
        {
            cch::byte rawHash[32];

            std::array<std::uint_fast32_t, 8> parts =
            {
                0x6a09e667,
                0xbb67ae85,
                0x3c6ef372,
                0xa54ff53a,
                0x510e527f,
                0x9b05688c,
                0x1f83d9ab,
                0x5be0cd19
            };
        };

        SHA256Hash() = default;

        explicit SHA256Hash(std::array<std::uint_fast32_t, 8> hash) :
            parts(hash) {}

        std::string toString() const
        {
            std::stringstream ss;

            if constexpr (std::endian::native == std::endian::big)
            {
                for (size_t i = 0; i < std::size(rawHash); ++i)
                {
                    ss << std::setw(2) << std::setfill('0') << std::hex << (static_cast<int>(rawHash[i]) & 0xFF);
                }
            }
            else
            {
                for (int i = 0; i < std::size(rawHash); i += 4)
                {
                    for (int j = i + 3; j >= i; --j)
                    {
                        ss << std::setw(2) << std::setfill('0') << std::hex << (static_cast<int>(rawHash[j]) & 0xFF);
                    }
                }
            }

            return ss.str();
        }

        std::span<cch::byte const> const getHash() const
        {
            return std::span<cch::byte const>(rawHash);
        }

        SHA256Hash& operator+=(SHA256Hash const& other)
        {
            for (size_t i = 0; i < std::size(parts); ++i)
            {
                parts[i] += other.parts[i];
            }

            return *this;
        }
    };

    class SHA256
    {
    public:
        static SHA256Hash hash(std::span<cch::byte> data);

    private:
        static void hash(std::span<cch::byte> data, SHA256Hash &hash, std::uint64_t inputDataSize);
        static void hashChunk(std::span<cch::byte> data, size_t chunkIdx, SHA256Hash& hashState);
        static SHA256Hash calculateHash(std::span<std::uint_fast32_t> data, SHA256Hash &hashState);

        static std::array<std::uint_fast32_t, 64> K;

        static std::uint_fast32_t s0(std::uint_fast32_t x);
        static std::uint_fast32_t s1(std::uint_fast32_t x);
    };
}
