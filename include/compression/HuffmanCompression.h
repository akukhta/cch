#pragma once
#include <vector>
#include <span>
#include <unordered_map>
#include <string>
#include <future>

namespace cch
{
    namespace compression
    {
        class HuffmanCompression
        {
        public:
            /// Compress the data
            /// \param data data to compress
            /// \param launchPolicy launch policy (async, sync)
            /// \return {Compression binary information to perform decompression, compressed data}
            std::future<std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> compress(std::span<unsigned char> data, std::launch launchPolicy);

            /// Decompress the data
            /// \param data {Compression binary information, compressed data}
            /// \param launchPolicy launch policy (async, sync)
            /// \return decompressed data
            std::future<std::vector<unsigned char>> decompress(std::pair<std::span<unsigned char>,std::span<unsigned char>> data, std::launch launchPolicy);

        private:

            /// Compression implementation
            /// \param data data to compress
            /// \return
            std::pair<std::vector<unsigned char>, std::vector<unsigned char>> compressData(std::span<unsigned char> data);
            std::vector<unsigned char> decompressData(std::pair<std::span<unsigned char>,std::span<unsigned char>> data);

            /// Calcalate frequency of each byte in a given buffer
            /// @param data buffer
            /// @return returns hash table where a[byte] = percentageOfEntries
            std::unordered_map<unsigned char, unsigned char> calculateCharFrequency(std::span<unsigned char> data);

            /// Huffman Three Node structure
            /// Designed To be stored as an array
            /// Nodes with indices [0; 255] are sym nodes (their index is equal to their value), the rest of the nodes are "immediate" nodes to generate the compression tree
            struct TreeNode
            {
                TreeNode() = default;
                TreeNode(unsigned int const weigth, int const left, int const right) : weigth(weigth), left(left), right(right) {}

                /// Weight
                unsigned int weigth = 0;
                int left = -1;
                int right = -1;
            };

            /// Builds the Huffman compression tree
            /// \param nodes vector of byte nodes [0, 255]
            /// \return an index of the tree root
            short buildTree(std::vector<TreeNode> &nodes);

            /// Creates and initializes the Huffman tree's nodes by their frequencies
            /// \param frequencyTable Frequency of each byte
            /// \return vector of nodes [0, 255]
            std::vector<TreeNode> initializeNodes(std::unordered_map<unsigned char, unsigned char> const &frequencyTable);

            /// Converts tree into a hash table to speed-up the lookup process
            /// \param codeTable Resulting code table
            /// \param nodes Huffman tree in array form
            /// \param nodeIdx index of a root node
            /// \param code Starting prefix
            void generateCodeTable(std::unordered_map<unsigned char, std::string> &codeTable, std::vector<TreeNode> const& nodes, short nodeIdx, std::string code = "");

            /// Generate ranges of used bytes (bytes that have non-zero frequency)
            /// \param frequencyTable
            /// \return Vector of ranges of used bytes (startOfRange, endOfRange)
            std::vector<std::pair<unsigned char, unsigned char>> generateCodeRanges(std::unordered_map<unsigned char, unsigned char> const &frequencyTable);

            /// Serialize byte frequencies, this data is being used for decompression
            /// \param rangesOfUsedBytes ranges of used bytes
            /// \param frequencyTable
            /// \return binary representation of used bytes and their frequencies
            std::vector<unsigned char> serializeBytesFrequencies(std::vector<std::pair<unsigned char, unsigned char>> const &rangesOfUsedBytes, std::unordered_map<unsigned char, unsigned char> const &frequencyTable);

            /// Deserialize frequency table
            /// \param data serialized frequency table
            /// \return frequency table
            std::unordered_map<unsigned char, unsigned char> restoreFrequencyTable(std::span<unsigned char> data);

            /// Helper function to invert code table used for compression (it has the following format {byte, compressedCode})
            /// to a table can be used for decompression ({compressedCode, originalByte})
            /// \param codeTable code table used for compression
            /// \return code table can be used for decompression
            std::unordered_map<size_t, unsigned char> inverseCodeTable(std::unordered_map<unsigned char, std::string> codeTable);
        };
    }
}
