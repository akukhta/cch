#include "compression/RLECompression.h"
#include <limits>
#include "utilities/Utilities.h"

std::vector<char> cch::compression::RLECompression::compress(std::span<char> data)
{
    std::vector<char> compressedData;
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