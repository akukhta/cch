#include "../../include/compression/HuffmanCompression.h"
#include "../../include/utilities/bitstream.h"
#include <limits>
#include <queue>
#include <iostream>
#include <format>

std::vector<unsigned char> cch::compression::HuffmanCompression::compress(std::span<unsigned char> data)
{
    // Calculate the byte frequency
    auto byteFrequency = calculateCharFrequency(data);
    // Create TreeNodes, construct 256 elements (for byte 0x00-0xFF)
    std::vector<TreeNode> nodes(256);
    // Reserve memory for intermidiate nodes
    nodes.reserve(nodes.size() * 2);

    // Fill the frequency
    for (auto [byte, freq] : byteFrequency)
    {
        nodes[byte].weigth = freq;
    }

    // Deallocate memory
    byteFrequency.clear();

    // build tree
    auto rootIdx = buildTree(nodes);
    std::unordered_map<unsigned char, std::string> codeTable;
    generateCodeTable(codeTable, nodes, rootIdx);

    for (auto &[byte, code] : codeTable)
    {
        std::cout << std::format("{} : {}", static_cast<char>(byte), code) << std::endl;
    }

    obitstream<std::vector<unsigned char>> compressedData;

    for (auto byte : data)
    {
        auto code = codeTable[byte];

        for (auto bit : code)
        {
            compressedData.write((bit == '1'));
            std::cout << bit;
        }
    }

    return compressedData.extractBuffer();
}

std::vector<unsigned char> cch::compression::HuffmanCompression::decompress(std::span<unsigned char> data)
{
    return std::vector<unsigned char>{};
}

short cch::compression::HuffmanCompression::buildTree(std::vector<TreeNode> &nodes)
{
    auto comp = [&nodes](short a, short b)
    {
        return nodes[a].weigth > nodes[b].weigth;
    };

    std::priority_queue<short, std::deque<short>, decltype(comp)> pq{comp};

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        if (nodes[i].weigth > 0)
        {
            pq.push(static_cast<short>(i));
        }
    }

    while (pq.size() != 1)
    {
        auto l = pq.top();
        pq.pop();

        auto r = pq.top();
        pq.pop();

        nodes.emplace_back(nodes[l].weigth + nodes[r].weigth, l, r);
        pq.push(static_cast<short>(nodes.size() - 1));
    }

    return pq.top();
}

void cch::compression::HuffmanCompression::generateCodeTable(
    std::unordered_map<unsigned char, std::string> &codeTable, std::vector<TreeNode> const &nodes, short nodeIdx, std::string code)
{
    if (nodes[nodeIdx].left == -1 || nodes[nodeIdx].right == -1)
    {
        codeTable[nodeIdx] = code;
        return;
    }
    else
    {
        generateCodeTable(codeTable, nodes, nodes[nodeIdx].left, code + "0");
        generateCodeTable(codeTable, nodes, nodes[nodeIdx].right, code + "1");
    }
}

std::vector<std::pair<short, short>> cch::compression::HuffmanCompression::generateCodeRanges(
    std::unordered_map<unsigned char, unsigned char> const &frequencyTable)
{
    return std::vector<std::pair<short, short>>{};
}

std::unordered_map<unsigned char, unsigned char> cch::compression::HuffmanCompression::calculateCharFrequency
    (std::span<unsigned char> data)
{
    std::unordered_map<unsigned char, size_t> frequencyTable;

    // Init table
    for (unsigned short i = 0; i <= std::numeric_limits<unsigned char>::max(); ++i)
    {
        frequencyTable[i] = 0;
    }

    auto increment = static_cast<unsigned char>(static_cast<int>(static_cast<double>(1 / data.size()) * 100));

    for (auto byte : data)
    {
        frequencyTable[byte]++;
    }

    std::unordered_map<unsigned char, unsigned char> frequencyTableCompact;

    for (auto [byte, freq] : frequencyTable)
    {
        frequencyTableCompact[byte]  = static_cast<unsigned char>((static_cast<double>(freq) / static_cast<double>(data.size())) * 100);
    }

    return frequencyTableCompact;
}
