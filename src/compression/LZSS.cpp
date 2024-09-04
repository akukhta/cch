#include "../include/compression/LZSS.h"
#include "../include/utilities/bitstream.h"

std::vector<cch::byte> cch::compression::LZSS::compress(std::span<cch::byte> data)
{
    // Encode data to vector of encoded elements

    std::vector<cch::compression::LZSS::EncodedElement> encodedElements;
    int inputSize = data.size();

    size_t currentPos = 0;

    while (currentPos < inputSize)
    {
        int matchLength = 0;
        int matchOffset = 0;

        // Find the longest match in the search buffer
        for (int offset = 1; offset <= std::min(cch::compression::LZSS::WINDOW_SIZE, currentPos); ++offset) {
            int length = 0;

            while (length < LOOK_AHEAD_BUFFER_SIZE && currentPos + length < inputSize &&
                data[currentPos + length] == data[currentPos - offset + length])
            {
                ++length;
            }

            if (length > matchLength)
            {
                matchLength = length;
                matchOffset = offset;
            }
        }

        if (matchLength >= MIN_MATCH_LENGTH)
        {
            EncodedElement element;
            element.isSingleByte = false;
            element.offset = matchOffset;
            element.length = matchLength;
            encodedElements.push_back(element);
            currentPos += matchLength;  // Advance by the length of the match
        }
        else
        {
            // Output a literal character
            EncodedElement element;
            element.isSingleByte = true;
            element.byte = data[currentPos];
            encodedElements.push_back(element);

            ++currentPos;  // Advance by one character
        }
    }

    return encodedElementsToRaw(encodedElements);
}

std::vector<cch::byte> cch::compression::LZSS::decompress(std::span<cch::byte> compressedData)
{
    std::vector<cch::byte> decoded;
    auto encodedElements = rawToEncodedElements(compressedData);

    for (const auto &element : encodedElements)
    {
        if (element.isSingleByte)
        {
            // Directly append the literal to the output
            decoded.push_back(element.byte);
        }
        else
        {
            // Copy the matched string from the output buffer
            int startPos = decoded.size() - element.offset;
            for (int i = 0; i < element.length; ++i)
            {
                decoded.push_back(decoded[startPos + i]);
            }
        }
    }

    return decoded;
}

std::vector<cch::byte> cch::compression::LZSS::encodedElementsToRaw(std::vector<EncodedElement> const &encoded)
{
    // 0 - sequence
    // 1 - literal
    obitstream<std::vector<cch::byte>> data;
    data.write(0, 8);

    for (auto &element : encoded)
    {
        data.write(element.isSingleByte);

        if (element.isSingleByte)
        {
            data.write(element.byte);
        }
        else
        {
            // offset - 1, length = 2
            data.write(element.offsetLengthSingleValue, 16);
        }
    }

    auto buffer = data.extractBuffer();
    buffer[0] = data.getBitsWrittenToCurrentByte();

    return buffer;
}

std::vector<cch::compression::LZSS::EncodedElement> cch::compression::LZSS::rawToEncodedElements(
    std::span<cch::byte> data)
{
    std::vector<EncodedElement> encodedElements;
    ibitstream in(data.begin(), data.end());

    char lastByteBitsUsed;
    in.readBits(lastByteBitsUsed,8);

    while (!in.eof() && lastByteBitsUsed > 0)
    {
        EncodedElement element;

        element.isSingleByte = in.read();

        if (element.isSingleByte)
        {
            in.readBits(element.byte, 8);
        }
        else
        {
            in.readBits(element.offsetLengthSingleValue, 16);
            auto someVForBreakpoint = rand() + rand();
        }

        if (in.isLastByte())
        {
            lastByteBitsUsed -= in.getBitsReadFromCurrentByte();
        }

        encodedElements.push_back(element);
    }

    return encodedElements;
}
