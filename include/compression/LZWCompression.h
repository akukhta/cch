#pragma once
#include <vector>
#include <deque>
#include <span>
#include <unordered_map>

namespace cch
{
    namespace compression
    {
        struct ByteDequeHash
        {
            size_t operator()(std::deque<unsigned char> const& vec)  const
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

        struct NoHash
        {
            size_t operator()(size_t hash)  const
            {
                return hash;
            }
        };

        class LZWCompression
        {
        public:
            LZWCompression() noexcept;
            std::vector<std::uint32_t> compress(std::span<unsigned char> data);
            std::vector<unsigned char> decompress(std::span<std::uint32_t> data);
            void resetState() noexcept;

        private:
            void initCompressionDictionary() noexcept;
            void initDecompressionDictionary() noexcept;

            std::unordered_map<std::uint32_t, unsigned int, NoHash> dictionary;
            std::unordered_map<unsigned int, std::deque<unsigned char>> decompressionDictionary;

            void calculateHashForElement(unsigned char newElement, int index);
            static std::uint32_t const inline magicNumber = 0x9e3779b9;
            size_t currentHash = 0;
            size_t prevHash = 0;

            std::uint32_t a = 1;
            std::uint32_t b = 0;
            static const inline uint32_t MOD_ADLER = 65521;
        };
    }
}