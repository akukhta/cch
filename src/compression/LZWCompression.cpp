#include "compression/LZWCompression.h"
#include <unordered_map>
#include <limits>

cch::compression::LZWCompression::LZWCompression()
{
    initDictionary();
    initDecompressionDictionary();
}

std::vector<size_t> cch::compression::LZWCompression::compress(std::span<unsigned char> data)
{
    std::vector<size_t> result;
    std::vector<unsigned char> currentByteSeq;
    int code = 256;

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
    int code = 256;
    std::vector<unsigned char> result;
    std::vector<unsigned char> currentByteSeq = decompressionDictionary[data[0]];

    auto appendVector = [](auto &src, auto& dst)
    {
        dst.insert(dst.end(), src.begin(), src.end());
    };

    appendVector(result, currentByteSeq);

    for (size_t i = 1; i < data.size(); ++i)
    {
        std::vector<unsigned char> entry;

        if (decompressionDictionary.find(data[i]) != decompressionDictionary.end())
        {
            entry = decompressionDictionary[data[i]];
        }
        else
        {
            entry = currentByteSeq;
            entry.push_back(currentByteSeq[0]);
        }

        appendVector(result, entry);
        decompressionDictionary[code++] = currentByteSeq;
        currentByteSeq = entry;
    }

    return result;
}

std::unordered_map<std::vector<unsigned char>, size_t, cch::compression::ByteVectorHash> & cch::compression::LZWCompression::getDictionary()
{
    return dictionary;
}

void cch::compression::LZWCompression::initDictionary()
{
    for (unsigned char i = 0; i != std::numeric_limits<unsigned char>::max(); ++i)
    {
        dictionary[std::vector<unsigned char>{i}] = i;
    }
}

void cch::compression::LZWCompression::initDecompressionDictionary()
{
    for (unsigned char i = 0; i != std::numeric_limits<unsigned char>::max(); ++i)
    {
        decompressionDictionary[i] = {i};
    }
}
