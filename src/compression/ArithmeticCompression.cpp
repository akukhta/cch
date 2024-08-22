#include "compression/ArithmeticCompression.h"

std::array<unsigned long, 256> cch::compression::ArithmeticCompression::countBytes(std::span<unsigned char> data)
{
    std::array<unsigned long, 256> counted;

    for (size_t i = 0; i < counted.size(); ++i)
    {
        counted[i] = 0;
    }

    for (auto c : data)
    {
        ++counted[c];
    }

    return counted;
}

std::array<unsigned char, 256> cch::compression::ArithmeticCompression::scaleCounts(const std::array<unsigned long, 256> &counts)
{
    std::array<unsigned char, 256> scaled;

    unsigned long maxCount = *std::max(counts.begin(), counts.end());
    unsigned int total = 1;
    unsigned long scale = maxCount / 256 + 1;

    for (size_t i = 0; i < counts.size(); ++i)
    {
        scaled[i] = static_cast<unsigned char>(counts[i] / scale);

        if (scaled[i] == 0 && counts[i] != 0)
        {
            scaled[i] = 1;
        }
    }

    for (size_t i = 0; i < counts.size(); ++i)
    {
        total += scaled[i];
    }

    if (total > (32767 - 256))
    {
        scale = 4;
    }
    else if (total > 16383)
    {
        scale = 2;
    }
    else
    {
        return scaled;
    }

    for (size_t i = 0; i < counts.size(); ++i)
    {
        scaled[i] /= scale;
    }

    return scaled;
}

void cch::compression::ArithmeticCompression::buildTotals(const std::array<unsigned char, 256> &scaled)
{
    totals[0] = 0;

    for (size_t i = 1; i < scaled.size(); ++i)
    {
        totals[i] = totals[i - 1] + scaled[i - 1];
    }

    totals[256] = totals[255] + 1;
}

void cch::compression::ArithmeticCompression::initializeArithmeticEncoder()
{
    low = 0;
    high = 0xFFFF;
    underflowBits = 0;
}

cch::compression::ArithmeticCompression::Symbol cch::compression::ArithmeticCompression::intToSymbol(int num)
{
    Symbol s;

    s.scale = totals[256];
    s.low_count = totals[num];
    s.high_count = totals[num + 1];

    return s;
}

void cch::compression::ArithmeticCompression::getSymbolScale(cch::compression::ArithmeticCompression::Symbol &s)
{
    s.scale = totals[256];
}

int cch::compression::ArithmeticCompression::symbolToInt(int count, cch::compression::ArithmeticCompression::Symbol &s)
{
    int c = 0;

    for (c = 256; count < totals[c]; --c);

    s.high_count = totals[c + 1];
    s.low_count = totals[c];

    return c;
}

unsigned char cch::compression::ArithmeticCompression::encodeSymbol(const cch::compression::ArithmeticCompression::Symbol &s)
{
    unsigned char res = 0x00;

    long range = static_cast<long>(high - low) + 1;

    high = low + static_cast<unsigned short>((range * s.high_count) / (s.scale - 1));
    low += static_cast<unsigned short>((range * s.low_count)/(s.scale));

    while (true)
    {
        if ((high & 0x8000) == (low & 0x8000))
        {
            res = high & 0x8000;

            while (underflowBits > 0)
            {
                res |= (~high & 0x8000);
                --underflowBits;
            }
        }
        else if ((low & 0x4000) && !(high & 0x4000))
        {
            ++underflowBits;
            low &= 0x3fff;
            high |= 0x4000;
        }
    }
    return 0;
}

