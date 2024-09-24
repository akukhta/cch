#include "../include/hash/SHA256.h"
#include <bit>
#include <assert.h>

std::array<std::uint_fast32_t, 64> cch::hash::SHA256::K =
    {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

cch::hash::SHA256Hash cch::hash::SHA256::hash(std::span<cch::byte> data)
{
    SHA256Hash hashState;
    hash(data, hashState, data.size());

    return hashState;
}

void cch::hash::SHA256::hash(std::span<cch::byte> data, SHA256Hash &hash, std::uint64_t inputDataSize)
{
    for (size_t i = 0; i < (data.size() * CHAR_BIT) / 512; ++i)
    {
        hashChunk({data.begin(), data.end()}, i, hash);
    }

    if ((data.size() * CHAR_BIT) % 512)
    {
        inputDataSize *= CHAR_BIT;

        std::vector<cch::byte> remainingBytes;
        size_t chunkStartIdx = data.size() - data.size() % 64;
        remainingBytes.reserve(data.size() - chunkStartIdx);

        std::copy(data.begin() + chunkStartIdx, data.end(), std::back_inserter(remainingBytes));

        remainingBytes.push_back(0x80);

        while (remainingBytes.size() * CHAR_BIT % 512 != 448)
        {
            remainingBytes.push_back(0x00);
        }

        if constexpr (std::endian::native == std::endian::little)
        {
            inputDataSize = std::byteswap(inputDataSize);
        }

        char const* dataSizePtr = reinterpret_cast<char const *>(&inputDataSize);
        remainingBytes.insert(remainingBytes.end(), dataSizePtr, dataSizePtr + sizeof(inputDataSize));

        for (size_t i = 0; i < (remainingBytes.size() * CHAR_BIT) / 512; ++i)
        {
            hashChunk({remainingBytes.begin(), remainingBytes.end()}, i, hash);
        }
    }
}

void cch::hash::SHA256::hashChunk(std::span<cch::byte> data, size_t chunkIdx, SHA256Hash &hashState)
{
    std::vector<cch::byte> block;
    block.reserve(64);
    std::copy(data.begin() + chunkIdx * 64, data.begin() + (chunkIdx + 1) * 64, std::back_inserter(block));

    assert(block.size() == 64);
    std::uint_fast32_t* int32Ptr = reinterpret_cast<std::uint_fast32_t*>(block.data());

    hashState += calculateHash({int32Ptr, block.size() / sizeof(std::uint_fast32_t)}, hashState);
}

cch::hash::SHA256Hash cch::hash::SHA256::calculateHash(std::span<std::uint_fast32_t> data, SHA256Hash &hashState)
{
    assert(data.size() * sizeof(std::uint_fast32_t) * CHAR_BIT == 512);

    std::array<std::uint_fast32_t, 64> w;
    std::copy(data.begin(), data.end(), w.begin());

    for (size_t i = 0; i < 16; ++i)
    {
        w[i] = std::byteswap(w[i]);
    }

    for (size_t i = 16; i < w.size(); ++i)
    {
        auto s0 = std::rotr(w[i - 15], 7) ^ std::rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        auto s1 = std::rotr(w[i - 2], 17) ^ std::rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    auto t = hashState.parts;

    auto& A = t[0];
    auto& B = t[1];
    auto& C = t[2];
    auto& D = t[3];
    auto& E = t[4];
    auto& F = t[5];
    auto& G = t[6];
    auto& H = t[7];

    for (size_t i = 0; i < 64; ++i)
    {
        auto S1 = std::rotr(E, 6) ^ std::rotr(E, 11) ^ std::rotr(E, 25);
        auto ch = (E & F) ^ ((~E) & G);
        auto tmp1 = H + S1 + ch + K[i] + w[i];
        auto S0 = std::rotr(A, 2) ^ std::rotr(A, 13) ^ std::rotr(A, 22);
        auto maj = (A & B) ^ (A & C) ^ (B & C);
        auto tmp2 = S0 + maj;

        H = G;
        G = F;
        F = E;
        E = D + tmp1;
        D = C;
        C = B;
        B = A;
        A = tmp1 + tmp2;
    }

    return SHA256Hash{t};
}

std::uint_fast32_t cch::hash::SHA256::s0(std::uint_fast32_t x)
{
    return std::rotr(x, 7) ^ std::rotr(x, 18) ^ (x >> 3);
}

std::uint_fast32_t cch::hash::SHA256::s1(std::uint_fast32_t x)
{
    return std::rotr(x, 17) ^ std::rotr(x, 19) ^ (x >> 10);
}
