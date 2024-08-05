#pragma once
#include <vector>
#include <span>
#include <unordered_map>

namespace cch
{
    namespace compression
    {
        struct ByteVectorHash
        {
            size_t operator()(std::vector<unsigned char> const& vec)  const
            {
                auto hash = vec.size();

                for (auto x : vec)
                {
                    hash ^= std::hash<unsigned char>()(x) + magicNumber + (hash << 6) + (hash >> 2);
                }

                return hash;
            }

            static std::uint32_t const inline magicNumber = 0x9e3779b9;
        };

        class LZWCompression
        {
        public:
            LZWCompression();
            std::vector<size_t> compress(std::span<unsigned char> data);
            std::vector<unsigned char> decompress(std::span<size_t> data);
            void resetState();

        private:
            void initCompressionDictionary();
            void initDecompressionDictionary();
            std::unordered_map<std::vector<unsigned char>, size_t, ByteVectorHash> dictionary;
            std::unordered_map<size_t, std::vector<unsigned char>> decompressionDictionary;
        };
    }
}