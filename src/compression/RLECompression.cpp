#include "compression/RLECompression.h"
#include <limits>
#include "utilities/Utilities.h"

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

        Utilities::addElementPerByte(count, compressedData);
        compressedData.push_back(data[i]);

        i = j;
    }

    return compressedData;
}

std::vector<unsigned char> cch::compression::RLECompression::decompress(std::span<unsigned char> data)
{
    std::vector<unsigned char> decompressedData;
    decompressedData.reserve(data.size());

    for (size_t i = 0; i < data.size() - 1; ++i)
    {
        unsigned char count = data[i];
        unsigned char byte = data[i + 1];

        for (size_t j = 0; j < count; ++j)
        {
            decompressedData.push_back(byte);
        }
    }

    return decompressedData;
}
