#pragma once
#include <span>
#include <array>

namespace cch::compression
{
    class ArithmeticCompression
    {
    public:
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
        std::array<int, 257> totals;

        void buildModel();
        void initializeArithmeticEncoder();
        std::array<unsigned long, 256> countBytes(std::span<unsigned char> data);
        std::array<unsigned char, 256> scaleCounts(std::array<unsigned long, 256> const& counts);
        void buildTotals(std::array<unsigned char, 256> const& scaled);
        Symbol intToSymbol(int num);
        void getSymbolScale(Symbol &s);
        int symbolToInt(int count, Symbol &s);
        unsigned char encodeSymbol(Symbol const &s);
    };
}