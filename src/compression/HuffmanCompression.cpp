#include "../../include/compression/HuffmanCompression.h"
#include "../../include/utilities/bitstream.h"
#include "../../include/hash/djb2hash.h"
#include <limits>
#include <queue>
#include <iostream>
#include <format>
#include <algorithm>

std::pair<std::vector<cch::byte>, std::vector<cch::byte>>  cch::compression::HuffmanCompression::compressData(std::span<cch::byte> data)
{
    // Calculate the byte frequency
    auto byteFrequency = calculateCharFrequency(data);
    // Initialize tree nodes
    auto nodes = initializeNodes(byteFrequency);

    // Serialize the frequency for used bytes only
    auto const usedRanges = generateCodeRanges(byteFrequency);
    auto serailizedFrequencies = serializeBytesFrequencies(usedRanges, byteFrequency);
    // Deallocate the memory
    byteFrequency.clear();

    // build tree
    auto const rootIdx = buildTree(nodes);
    // Build the code table for a given tree
    std::unordered_map<cch::byte, std::string> codeTable;
    generateCodeTable(codeTable, nodes, rootIdx);

    // Compress the data
    obitstream<std::vector<cch::byte>> compressedData;
    compressedData.getBuffer().reserve(data.size());

    for (auto byte : data)
    {
        auto const &code = codeTable[byte];

        for (auto const bit : code)
        {
            compressedData.write(bit == '1');
        }
    }

    serailizedFrequencies[0] = compressedData.getBitsWrittenToCurrentByte();
    compressedData.getBuffer().shrink_to_fit();

    return std::make_pair(std::move(serailizedFrequencies), std::move(compressedData.extractBuffer()));
}

std::vector<cch::byte> cch::compression::HuffmanCompression::decompressData(
    std::pair<std::span<cch::byte>, std::span<cch::byte>> data)
{
    std::vector<cch::byte> decompressedData;
    decompressedData.reserve(data.second.size() * 1.3);

    // Restore the frequency table from the compression info
    auto const frequencyTable = restoreFrequencyTable(data.first);

    // Initialize nodes
    auto nodes = initializeNodes(frequencyTable);
    // Build the tree
    auto const rootIdx = buildTree(nodes);

    std::unordered_map<size_t, cch::byte> codeTable;

    {
        // Generate the code table & inverse it (from byte->code to code->byte)
        std::unordered_map<cch::byte, std::string> ccodeTable;
        ccodeTable.reserve(256);
        generateCodeTable(ccodeTable, nodes, rootIdx);
        codeTable = inverseCodeTable(std::move(ccodeTable));
    }

    // Get the amount of bits used in the last byte of decompressed data
    char lastByteBits = static_cast<char>(data.first[0]);

    // Decompress the data
    ibitstream in(data.second.begin(), data.second.end());
    size_t currentHash = cch::hash::djb2::defaultHashValue;

    while (lastByteBits > 0 && !in.eof())
    {
        currentHash = cch::hash::djb2::djb2hash(currentHash, in.read() ? '1' : '0');

        lastByteBits -= in.isLastByte() ? 1 : 0;

        if (auto it = codeTable.find(currentHash); it != codeTable.end())
        {
            decompressedData.push_back(it->second);
            currentHash = cch::hash::djb2::defaultHashValue;
        }
    }

    return decompressedData;
}

short cch::compression::HuffmanCompression::buildTree(std::vector<TreeNode> &nodes)
{
    auto comp = [&nodes](short const a, short const b)
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
    std::unordered_map<cch::byte, cch::byte> const &frequencyTable)
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
    std::unordered_map<cch::byte, std::string> &codeTable, std::vector<TreeNode> const &nodes, short const nodeIdx, std::string code)
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

    generateCodeTable(codeTable, nodes, nodes[nodeIdx].left, code + "0");
    generateCodeTable(codeTable, nodes, nodes[nodeIdx].right, code + "1");
}

std::vector<std::pair<cch::byte, cch::byte>> cch::compression::HuffmanCompression::generateCodeRanges(
    std::unordered_map<cch::byte, cch::byte> const &frequencyTable)
{
    std::vector<std::pair<cch::byte, cch::byte>> usedRanges;

    // Fill vector of used bytes
    std::vector<cch::byte> indices;

    for (auto [byte, f] : frequencyTable)
    {
        if (f > 0)
        {
            indices.push_back(byte);
        }
    }

    std::ranges::sort(indices, std::less<>());

    // Convert vector of used bytes to ranges format (FirstUsedByte, LastUsedByte)
    for (size_t i = 0; i < indices.size();)
    {
        cch::byte start = indices[i];
        cch::byte end = start;
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

std::vector<cch::byte> cch::compression::HuffmanCompression::serializeBytesFrequencies(
    std::vector<std::pair<cch::byte, cch::byte>> const &rangesOfUsedBytes,
    std::unordered_map<cch::byte, cch::byte> const &frequencyTable)
{
    std::vector<cch::byte> serializedByteFrequency;
    serializedByteFrequency.reserve(512);

    // Reserve the first byte to store the amount of bits used in the last byte of the compressed data
    serializedByteFrequency.push_back(0x00);

    for (auto [rangeStart, rangeEnd] : rangesOfUsedBytes)
    {
        serializedByteFrequency.push_back(rangeStart);
        serializedByteFrequency.push_back(rangeEnd);

        for (unsigned short idx = rangeStart; idx <= rangeEnd; ++idx)
        {
            serializedByteFrequency.push_back(frequencyTable.find(idx)->second);
        }
    }

    serializedByteFrequency.shrink_to_fit();
    return serializedByteFrequency;
}

std::unordered_map<cch::byte, cch::byte> cch::compression::HuffmanCompression::restoreFrequencyTable(
    std::span<cch::byte> const data)
{
    std::unordered_map<cch::byte, cch::byte> byteFrequency;
    byteFrequency.reserve(256);

    for (size_t i = 1; i < data.size() - 1;)
    {
        unsigned short const startOfRange = data[i];
        unsigned short const endOfRange = data[i + 1];

        for (size_t c = 0; c <= (endOfRange - startOfRange); ++c)
        {
            byteFrequency[startOfRange + c] = data[i + 2 + c];
        }

        i += 3 + (endOfRange - startOfRange);
    }

    return byteFrequency;
}

std::unordered_map<size_t, cch::byte> cch::compression::HuffmanCompression::inverseCodeTable(
    std::unordered_map<cch::byte, std::string> codeTable)
{
    std::unordered_map<size_t, cch::byte> inversedTable;
    inversedTable.reserve(codeTable.size());

    for (auto &[byte, code] : codeTable)
    {
        inversedTable[cch::hash::djb2::djb2hash(code)] = byte;
    }

    return inversedTable;
}

std::unordered_map<cch::byte, cch::byte> cch::compression::HuffmanCompression::calculateCharFrequency
    (std::span<cch::byte> const data)
{
    std::unordered_map<cch::byte, size_t> frequencyTable;
    frequencyTable.reserve(256);

    // Init table
    for (unsigned short i = 0; i <= std::numeric_limits<cch::byte>::max(); ++i)
    {
        frequencyTable[i] = 0;
    }

    for (auto byte : data)
    {
        frequencyTable[byte]++;
    }

    size_t max = 0;

    for (auto [b, f] : frequencyTable)
    {
        max = std::max(f, max);
    }

    if (max == 0)
    {
        frequencyTable[0] = 1;
        max = 1;
    }

    max = (max / 255) + 1;

    std::unordered_map<cch::byte, cch::byte> frequencyTableCompact;
    frequencyTableCompact.reserve(256);

    for (auto [byte, freq] : frequencyTable)
    {
        frequencyTableCompact[byte] = static_cast<unsigned int>(freq / max);

        if (frequencyTableCompact[byte] == 0 && freq != 0)
        {
            frequencyTableCompact[byte] = 1;
        }
    }

    return frequencyTableCompact;
}

int t(int x)
{
    return x * x;
}

std::future<std::pair<std::vector<cch::byte>, std::vector<cch::byte>>>
cch::compression::HuffmanCompression::compress(std::span<cch::byte> data, std::launch launchPolicy)
{
    return std::async(launchPolicy, &cch::compression::HuffmanCompression::compressData, this, data);
}

std::future<std::vector<cch::byte>>
cch::compression::HuffmanCompression::decompress(std::pair<std::span<cch::byte>, std::span<cch::byte>> data, std::launch launchPolicy)
{
    return std::async(launchPolicy, &cch::compression::HuffmanCompression::decompressData, this, data);
}
