#pragma once
#include <array>
#include <span>
#include <vector>

namespace cch::compression
{
    class LZSS
    {
    public:
        std::vector<unsigned char> compress(std::span<unsigned char> data);
        std::vector<unsigned char> decompress(std::span<unsigned char> compressedData);

    private:
        struct EncodedElement
        {
            bool isSingleByte;

            union
            {
                unsigned char byte;

                union
                {
                    struct
                    {
                        unsigned short offset : 12;
                        unsigned short length : 4;
                    };

                    unsigned short offsetLengthSingleValue;
                };
            };
        };

        std::vector<unsigned char> encodedElementsToRaw(std::vector<EncodedElement> const &encoded);
        std::vector<EncodedElement> rawToEncodedElements(std::span<unsigned char> data);

        static size_t const inline WINDOW_SIZE = 4096;   // Size of the search buffer (also known as the sliding window)
        static size_t const inline LOOK_AHEAD_BUFFER_SIZE = 18;  // Size of the look-ahead buffer
        static size_t const inline INDEX_BIT_COUNT = 12;
        static size_t const inline LENGTH_BIT_COUNT = 4;
        //static size_t const inline MIN_MATCH_LENGTH = (1 + INDEX_BIT_COUNT + LENGTH_BIT_COUNT) / 9;   // Minimum match length to be considered for compression
        static size_t const inline MIN_MATCH_LENGTH = 3;   // Minimum match length to be considered for compression
        static size_t const inline MAX_MATCH_LENGTH = (1 << LENGTH_BIT_COUNT) - 1;  // Maximum match length (due to look-ahead buffer size)


    };
}
