#include "compression/LZWCompression.h"
#include <unordered_map>
#include <limits>
#include <deque>
#include <limits>

cch::compression::LZWCompression::LZWCompression() noexcept
{
    resetState();
}

std::vector<std::uint32_t> cch::compression::LZWCompression::compress(std::span<unsigned char> data)
{
    std::vector<std::uint32_t> result;
    result.reserve(data.size() / sizeof(unsigned int));

    std::deque<unsigned char> currentByteSeq;
    int code = dictionary.size();

    size_t i = 0;

    for (auto x : data)
    {
        currentByteSeq.push_back(x);

        calculateHashForElement(x, currentByteSeq.size());

        if (dictionary.find(currentHash) == dictionary.end())
        {
            auto bytePrefix = std::move(currentByteSeq);
            bytePrefix.pop_back();
            result.push_back(dictionary[prevHash]);

            if (dictionary.size() < std::numeric_limits<std::uint32_t>::max())
            {
                dictionary[currentHash] = code++;
            }

            currentByteSeq.clear();
            currentByteSeq.push_back(x);

            currentHash = 0;
            a = 1;
            b = 0;

            calculateHashForElement(x, currentByteSeq.size());
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
        std::deque<unsigned char> entry;

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

        if (decompressionDictionary.size() < std::numeric_limits<std::uint32_t>::max())
        {
            decompressionDictionary[code++] = currentByteSeq;
        }

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
        a = 1;
        b = 0;

        calculateHashForElement(i, 1);
        dictionary[currentHash] = i;
    }

    currentHash = 0;
    a = 1;
    b = 0;
}

void cch::compression::LZWCompression::initDecompressionDictionary() noexcept
{
    decompressionDictionary.clear();

    for (unsigned int i = 0; i <= std::numeric_limits<unsigned char>::max(); ++i)
    {
        decompressionDictionary[i] = { static_cast<unsigned char>(i) };
    }
}

void cch::compression::LZWCompression::calculateHashForElement(unsigned char newElement, int index)
{
    a = (a + newElement) % MOD_ADLER;
    b = (b + a) % MOD_ADLER;
    //currentHash = currentHash * 31 + newElement;
    currentHash = std::hash<size_t>()((b << 16) | a);
    //currentHash ^= std::hash<size_t>()(static_cast<size_t>(newElement) * index);
}