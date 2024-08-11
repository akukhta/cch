#include "../../include/compression/HuffmanCompression.h"
#include "../../include/utilities/bitstream.h"
#include <limits>
#include <queue>
#include <iostream>
#include <format>
#include <algorithm>

std::pair<std::vector<unsigned char>, std::vector<unsigned char>>  cch::compression::HuffmanCompression::compress(std::span<unsigned char> data)
{
    // Calculate the byte frequency
    auto byteFrequency = calculateCharFrequency(data);
    auto nodes = initializeNodes(byteFrequency);

    // Deallocate memory
    auto usedRanges = generateCodeRanges(byteFrequency);
    auto serializedTree = serializeTree(usedRanges, byteFrequency);
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

    std::cout << std::endl;

    serializedTree[0] = compressedData.getBitsWrittenToCurrentByte();

    return std::make_pair(std::move(serializedTree), std::move(compressedData.extractBuffer()));
}

std::vector<unsigned char> cch::compression::HuffmanCompression::decompress(
    std::pair<std::span<unsigned char>, std::span<unsigned char>> data)
{
    std::vector<unsigned char> decompressedData;

    auto frequencyTable = restoreFrequencyTable(data.first);

    auto nodes = initializeNodes(frequencyTable);
    auto rootIdx = buildTree(nodes);
    std::unordered_map<std::string, unsigned char> codeTable;

    {
        std::unordered_map<unsigned char, std::string> ccodeTable;
        generateCodeTable(ccodeTable, nodes, rootIdx);
        codeTable = inverseCodeTable(std::move(ccodeTable));

        std::cout << "Restored code table:" << std::endl;

        for (auto &[code, byte] : codeTable)
        {
            std::cout << std::format("{} : {}", code, static_cast<char>(byte)) << std::endl;
        }
    }

    ibitstream in(data.second.begin(), data.second.end());

    std::string currentCode;
    char lastByteBits = data.first[0];

    std::cout << "Decompressed Data:" << std::endl;

    while (lastByteBits > 0 && !in.eof())
    {
        currentCode += in.read() ? "1" : "0";
        lastByteBits -= in.isLastByte() ? 1 : 0;

        if (codeTable.find(currentCode) != codeTable.end())
        {
            auto byte = codeTable[currentCode];
            std::cout << static_cast<char>(byte);
            decompressedData.push_back(codeTable[currentCode]);
            currentCode = "";
        }
    }

    return decompressedData;
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

std::vector<cch::compression::HuffmanCompression::TreeNode> cch::compression::HuffmanCompression::initializeNodes(
    std::unordered_map<unsigned char, unsigned char> const &frequencyTable)
{
    // Create TreeNodes, construct 256 elements (for byte 0x00-0xFF)
    std::vector<TreeNode> nodes(256);
    // Reserve memory for intermidiate nodes
    nodes.reserve(nodes.size() * 2);

    // Fill the frequency
    for (auto [byte, freq] : frequencyTable)
    {
        nodes[byte].weigth = freq;
    }

    return nodes;
}

void cch::compression::HuffmanCompression::generateCodeTable(
    std::unordered_map<unsigned char, std::string> &codeTable, std::vector<TreeNode> const &nodes, short nodeIdx, std::string code)
{
    if (nodes[nodeIdx].left == -1 || nodes[nodeIdx].right == -1)
    {
        if (code.empty())
        {
            code = "0";
        }

        codeTable[nodeIdx] = code;
        return;
    }
    else
    {
        generateCodeTable(codeTable, nodes, nodes[nodeIdx].left, code + "0");
        generateCodeTable(codeTable, nodes, nodes[nodeIdx].right, code + "1");
    }
}

std::vector<std::pair<unsigned char, unsigned char>> cch::compression::HuffmanCompression::generateCodeRanges(
    std::unordered_map<unsigned char, unsigned char> const &frequencyTable)
{
    std::vector<std::pair<unsigned char, unsigned char>> usedRanges;

    // Fill vector of used bytes
    std::vector<unsigned char> indices;

    for (auto [byte, f] : frequencyTable)
    {
        if (f > 0)
        {
            indices.push_back(byte);
        }
    }

    std::sort(indices.begin(), indices.end(), std::less<unsigned char>());

    // Convert vector of used bytes to ranges format (FirstUsedByte, LastUsedByte)
    for (size_t i = 0; i < indices.size(); ++i)
    {
        unsigned char start = indices[i];
        unsigned char end = start;
        size_t j = i + 1;

        while (j < indices.size())
        {
            if (end + 1 == indices[j])
            {
                end = indices[j++];
            }
            else
            {
                break;
            }
        }

        usedRanges.emplace_back(start, end);

        i = j;
    }

    return usedRanges;
}

std::vector<unsigned char> cch::compression::HuffmanCompression::serializeTree(
    std::vector<std::pair<unsigned char, unsigned char>> const &rangesOfUsedBytes,
    std::unordered_map<unsigned char, unsigned char> const &frequencyTable)
{
    std::vector<unsigned char> serializedTree;
    // Reserve the first byte to store the amount of bits used in the last byte of the compressed data
    serializedTree.push_back(0x00);

    for (auto &range : rangesOfUsedBytes)
    {
        serializedTree.push_back(range.first);
        serializedTree.push_back(range.second);

        for  (unsigned char idx = range.first; idx <= range.second; ++idx)
        {
            serializedTree.push_back(frequencyTable.find(idx)->second);
        }
    }

    return serializedTree;
}

std::unordered_map<unsigned char, unsigned char> cch::compression::HuffmanCompression::restoreFrequencyTable(
    std::span<unsigned char> data)
{
    std::unordered_map<unsigned char, unsigned char> byteFrequency;

    for (size_t i = 1; i < data.size() - 1;)
    {
        unsigned char startOfRange = data[i];
        unsigned char endOfRange = data[i + 1];

        for (size_t c = 0, j = i + 2; j <= i + 2 + (endOfRange - startOfRange); ++j, ++c)
        {
            byteFrequency[startOfRange + c] = data[j];
        }

        i += 2 + (endOfRange - startOfRange);
    }

    return byteFrequency;
}

std::unordered_map<std::string, unsigned char> cch::compression::HuffmanCompression::inverseCodeTable(
    std::unordered_map<unsigned char, std::string> codeTable)
{
    std::unordered_map<std::string, unsigned char> inversedTable;
    inversedTable.reserve(codeTable.size());

    for (auto &[byte, code] : codeTable)
    {
        inversedTable[code] = byte;
    }

    return inversedTable;
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
