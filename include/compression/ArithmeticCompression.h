#pragma once
#include <span>
#include <array>
#include <vector>
#include "../utilities/bitstream.h"

namespace cch::compression
{
    class ArithmeticCompression
    {
    public:
        std::vector<unsigned char> compress(std::span<unsigned char> data);

    private:
        struct Symbol
        {
            unsigned short low_count;
            unsigned short high_count;
            unsigned short scale;
        };

        unsigned short code;
        unsigned short low;
        unsigned short high;
        long underflowBits;

        void buildModel(std::span<unsigned char> data);
        void initializeArithmeticEncoder();
        void flushEncoder();
        std::array<unsigned long, 256> countBytes(std::span<unsigned char> data);
        std::array<unsigned char, 256> scaleCounts(std::array<unsigned long, 256> const& counts);
        void buildTotals(std::array<unsigned char, 256> const& scaled);
        Symbol intToSymbol(int num);
        void getSymbolScale(Symbol &s);
        int symbolToInt(int count, Symbol &s);
        void encodeSymbol(Symbol const &s);

        std::array<short, 258> totals;

        static size_t const inline END_OF_STREAM = 257;
        obitstream<std::vector<unsigned char>> out;
    };
}