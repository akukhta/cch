#include "compression/LZWCompression.h"
#include <unordered_map>
#include <limits>
#include <deque>

cch::compression::LZWCompression::LZWCompression() noexcept
{
    resetState();
}

std::vector<unsigned int> cch::compression::LZWCompression::compress(std::span<unsigned char> data)
{
    std::vector<unsigned int> result;
    result.reserve(data.size() / sizeof(unsigned int));

    std::deque<unsigned char> currentByteSeq;
    int code = dictionary.size();

    size_t i = 0;

    for (auto x : data)
    {
        currentByteSeq.push_back(x);

        calculateHashForElement(x);

        if (dictionary.find(currentHash) == dictionary.end())
        {
            auto bytePrefix = std::move(currentByteSeq);
            bytePrefix.pop_back();
            result.push_back(dictionary[prevHash]);
            dictionary[currentHash] = code++;

            currentByteSeq.clear();
            currentByteSeq.push_back(x);

            currentHash = 0;
            calculateHashForElement(x);
        }

        prevHash = currentHash;
        ++i;
    }

    if (!currentByteSeq.empty())
    {
        result.push_back(dictionary[currentHash]);
    }

    result.shrink_to_fit();
    return result;
}

std::vector<unsigned char> cch::compression::LZWCompression::decompress(std::span<unsigned int> data)
{
    std::vector<unsigned char> result;
    int code = decompressionDictionary.size();

    auto currentByteSeq = decompressionDictionary[data[0]];

    auto appendVector = [](auto& dst, auto& src)
        {
            dst.insert(dst.end(), src.begin(), src.end());
        };

    appendVector(result, currentByteSeq);

    for (size_t i = 1; i < data.size(); ++i)
    {
        decltype(currentByteSeq) entry;

        if (auto it = decompressionDictionary.find(data[i]); it != decompressionDictionary.end())
        {
            entry = it->second;
        }
        else if (data[i] == code)
        {
            entry = currentByteSeq;
            entry.push_back(currentByteSeq[0]);
        }

        appendVector(result, entry);

        currentByteSeq.push_back(entry[0]);
        decompressionDictionary[code++] = currentByteSeq;
        currentByteSeq = entry;
    }

    return result;
}

void cch::compression::LZWCompression::resetState() noexcept
{
    initCompressionDictionary();
    initDecompressionDictionary();
}


void cch::compression::LZWCompression::initCompressionDictionary() noexcept
{
    dictionary.clear();

    for (unsigned int i = 0; i <= std::numeric_limits<unsigned char>::max(); ++i)
    {
        currentHash = 0;
        calculateHashForElement(i);
        dictionary[currentHash] = i;
    }

    currentHash = 0;
}

void cch::compression::LZWCompression::initDecompressionDictionary() noexcept
{
    decompressionDictionary.clear();

    for (unsigned int i = 0; i <= std::numeric_limits<unsigned char>::max(); ++i)
    {
        decompressionDictionary[i] = { static_cast<unsigned char>(i) };
    }
}

void cch::compression::LZWCompression::calculateHashForElement(unsigned char newElement)
{
    currentHash = std::hash<size_t>()(currentHash ^ std::hash<unsigned char>()(newElement));
}