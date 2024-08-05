#include "compression/RLECompression.h"
#include <limits>
#include "utilities/Utilities.h"

std::vector<char> cch::compression::RLECompression::compress(std::span<char> data)
{
    std::vector<char> compressedData;
    compressedData.reserve(data.size());

    for (size_t i = 0; i < data.size(); i++)
    {
        unsigned char count = 1;
        size_t j = i + 1;

        while (data[i] == data[j++])
        {
            ++count;

            if (count == std::numeric_limits<decltype(count)>::max())
            {
                break;
            }
        }

        i = j;

        Utilities::addElementPerByte(count, compressedData);
    }

    return compressedData;
}
