#include "compression/RLECompression.h"
#include "utilities/Utilities.h"
#include <limits>
#include <stdexcept>

std::vector<unsigned char> cch::compression::RLECompression::compress(std::span<unsigned char> data)
{
    std::vector<unsigned char> compressedData;
    compressedData.reserve(data.size());

    for (size_t i = 0; i < data.size();)
    {
        unsigned char count = 1;
        size_t j = i + 1;

        while (j < data.size() && data[i] == data[j])
        {
            ++count;

            if (count == std::numeric_limits<decltype(count)>::max())
            {
                break;
            }

            ++j;
        }

        compressedData.push_back(data[i]);
        Utilities::addElementPerByte(count, compressedData);

        i = j;
    }

    return compressedData;
}

std::vector<unsigned char> cch::compression::RLECompression::decompress(std::span<unsigned char> data)
{
    if (data.size() % 2 == 1)
    {
        throw std::runtime_error("The data size should be dividable by 2 (byte, count)");
    }

    std::vector<unsigned char> decompressedData;
    decompressedData.reserve(data.size());

    for (size_t i = 0; i < data.size() - 1; i += 2)
    {
        unsigned char byte = data[i];
        unsigned char count = data[i + 1];

        for (unsigned char j = 0; j < count; ++j)
        {
            decompressedData.push_back(byte);
        }
    }

    return decompressedData;
}