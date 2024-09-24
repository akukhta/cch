#pragma once
#include <span>
#include <sstream>
#include <iomanip>
#include <vector>
#include <bit>

#include "config/types.h"

namespace cch::hash
{
    struct SHA512Hash
    {
        union
        {
            cch::byte rawHash[64];

            std::array<std::uint_fast64_t, 8> parts =
            {
                0x6a09e667f3bcc908,
                0xbb67ae8584caa73b,
                0x3c6ef372fe94f82b,
                0xa54ff53a5f1d36f1,
                0x510e527fade682d1,
                0x9b05688c2b3e6c1f,
                0x1f83d9abfb41bd6b,
                0x5be0cd19137e2179
            };
        };

        SHA512Hash() = default;

        explicit SHA512Hash(std::array<std::uint_fast64_t, 8> hash) :
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
                for (int i = 0; i < std::size(rawHash); i += 8)
                {
                    for (int j = i + 7; j >= i; --j)
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

        SHA512Hash& operator+=(SHA512Hash const& other)
        {
            for (size_t i = 0; i < std::size(parts); ++i)
            {
                parts[i] += other.parts[i];
            }

            return *this;
        }

        // explicit SHA256Hash(size_t inputDataSize = 0) : A(A0), B(B0), C(C0), D(D0) {}
        //
        // SHA256Hash(std::uint_fast32_t A, uint_fast32_t B, uint_fast32_t C, uint_fast32_t D) :
        //     A(A), B(B), C(C), D(D) {}
        //
        // static std::uint_fast32_t const inline A0 = 0x67452301;
        // static std::uint_fast32_t const inline B0 = 0xefcdab89;
        // static std::uint_fast32_t const inline C0 = 0x98badcfe;
        // static std::uint_fast32_t const inline D0 = 0x10325476;
    };

    class SHA512
    {
    public:
        static SHA512Hash hash(std::vector<cch::byte> data);
        explicit SHA512(size_t inputDataSize) : inputDataSize(inputDataSize) {}

        SHA512Hash hashChunk(std::vector<cch::byte> data);
    private:
        static void hash(std::vector<cch::byte> data, SHA512Hash &hash, std::uint64_t inputDataSize);
        static void hashChunk(std::span<cch::byte> data, size_t chunkIdx, SHA512Hash& hashState);
        static SHA512Hash calculateHash(std::span<std::uint_fast64_t> data, SHA512Hash &hashState);

        static std::array<std::uint_fast64_t, 80> K;

        SHA512Hash currentHashState;
        size_t inputDataSize = 0;
    };
}
