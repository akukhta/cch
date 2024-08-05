#include "compression/LZWCompression.h"
#include <unordered_map>
#include <limits>

cch::compression::LZWCompression::LZWCompression()
{
    resetState();
}

std::vector<size_t> cch::compression::LZWCompression::compress(std::span<unsigned char> data)
{
    std::vector<size_t> result;
    std::vector<unsigned char> currentByteSeq;
    int code = dictionary.size();

    for (auto x : data)
    {
        currentByteSeq.push_back(x);

        if (dictionary.find(currentByteSeq) == dictionary.end())
        {
            auto bytePrefix = currentByteSeq;
            bytePrefix.pop_back();
            result.push_back(dictionary[bytePrefix]);
            dictionary[currentByteSeq] = code++;
            currentByteSeq = {x};
        }
    }

    if (!currentByteSeq.empty())
    {
        result.push_back(dictionary[currentByteSeq]);
    }

    return result;
}

std::vector<unsigned char> cch::compression::LZWCompression::decompress(std::span<size_t> data)
{
    std::vector<unsigned char> result;
    int code = decompressionDictionary.size();

    std::vector<unsigned char> currentByteSeq = decompressionDictionary[data[0]];

    auto appendVector = [](auto &dst, auto& src)
    {
        dst.insert(dst.end(), src.begin(), src.end());
    };

    appendVector(result, currentByteSeq);

    for (size_t i = 1; i < data.size(); ++i)
    {
        std::vector<unsigned char> entry;

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

void cch::compression::LZWCompression::resetState()
{
    initCompressionDictionary();
    initDecompressionDictionary();
}


void cch::compression::LZWCompression::initCompressionDictionary()
{
    dictionary.clear();

    for (unsigned char i = 0; i != std::numeric_limits<unsigned char>::max(); ++i)
    {
        dictionary[std::vector<unsigned char>{i}] = i;
    }
}

void cch::compression::LZWCompression::initDecompressionDictionary()
{
    decompressionDictionary.clear();

    for (unsigned char i = 0; i != std::numeric_limits<unsigned char>::max(); ++i)
    {
        decompressionDictionary[i] = {i};
    }
}