#include "compression/RLECompression.h"
#include "utilities/Utilities.h"
#include <limits>
#include <stdexcept>
#include <iterator>

std::vector<unsigned char> cch::compression::RLECompression::decompress(std::span<unsigned char> data)
{
    std::vector<unsigned char> decompressedData;
    decompressedData.reserve(data.size() * 2);
    
    for (size_t i = 0; i < data.size(); ++i) 
    {
        char countOfbytes = static_cast<char>(data[i]);

        if (countOfbytes >= 0)
        {
            decompressedData.insert(decompressedData.end(), data.begin() + i + 1, data.begin() + i + 1 + countOfbytes + 1);
            i += countOfbytes + 1;
        }
        else if (countOfbytes < 0)
        {
            char repeatedByte = data[i + 1];
            std::fill_n(std::back_inserter(decompressedData), abs(countOfbytes) + 1, repeatedByte);
            i += 1;
        }
    }

    decompressedData.shrink_to_fit();
    return decompressedData;
}

std::vector<unsigned char> cch::compression::RLECompression::compress(std::span<unsigned char> data)
{
    std::vector<unsigned char> compressed;
    compressed.reserve(data.size());

    for (size_t i = 0; i < data.size(); ) 
    {
        size_t start = i;

        if (i == data.size() - 1 || data[i] == data[i + 1])
        {
            while (i < data.size() && data[i] == data[start] && (i - start) < 128)
            {
                ++i;
            }

            compressed.push_back(static_cast<uint8_t>(-(i - start) + 1));
            compressed.push_back(data[start]);

        }
        else
        {
            while (i < data.size() && (i == start || data[i] != data[i - 1]) && (i - start) < 128) 
            {
                ++i;
            }

            size_t length = i - start;
            compressed.push_back(static_cast<unsigned char>(length - 1));
            compressed.insert(compressed.end(), data.begin() + start, data.begin() + start + length);
        }
    }

    compressed.shrink_to_fit();
    return compressed;
}

