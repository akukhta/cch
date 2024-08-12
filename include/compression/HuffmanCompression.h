#pragma once
#include <vector>
#include <span>
#include <unordered_map>
#include <string>

namespace cch
{
    namespace compression
    {
        class HuffmanCompression
        {
        public:
            std::pair<std::vector<unsigned char>, std::vector<unsigned char>> compress(std::span<unsigned char> data);
            std::vector<unsigned char> decompress(std::pair<std::span<unsigned char>,std::span<unsigned char>> data);

        private:
            /// Calcalate frequency of each byte in a given buffer
            /// @param data buffer
            /// @return returns hash table where a[byte] = percentageOfEntries
            std::unordered_map<unsigned char, unsigned char> calculateCharFrequency(std::span<unsigned char> data);

            struct TreeNode
            {
                TreeNode() = default;
                TreeNode(unsigned int const weigth, int const left, int const right) : weigth(weigth), left(left), right(right) {}

                unsigned int weigth = 0;
                int left = -1;
                int right = -1;
            };

            short buildTree(std::vector<TreeNode> &nodes);
            std::vector<TreeNode> initializeNodes(std::unordered_map<unsigned char, unsigned char> const &frequencyTable);

            void generateCodeTable(std::unordered_map<unsigned char, std::string> &codeTable, std::vector<TreeNode> const& nodes, short nodeIdx, std::string code = "");
            std::vector<std::pair<unsigned char, unsigned char>> generateCodeRanges(std::unordered_map<unsigned char, unsigned char> const &frequencyTable);
            std::vector<unsigned char> serializeBytesFrequencies(std::vector<std::pair<unsigned char, unsigned char>> const &rangesOfUsedBytes, std::unordered_map<unsigned char, unsigned char> const &frequencyTable);

            std::unordered_map<unsigned char, unsigned char> restoreFrequencyTable(std::span<unsigned char> data);
            std::unordered_map<size_t, unsigned char> inverseCodeTable(std::unordered_map<unsigned char, std::string> codeTable);

            static size_t djb2hash(std::string const& str);
            static size_t djb2hash(size_t prevHash, char c);
            static size_t const inline defaultHashValue = 5381;
        };
    }
}
