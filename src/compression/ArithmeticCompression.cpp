#include "compression/ArithmeticCompression.h"
#include <algorithm>

/// Implemented
std::array<unsigned long, 256> cch::compression::ArithmeticCompression::countBytes(std::span<cch::byte> data)
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

/// Implemented
std::array<cch::byte, 256> cch::compression::ArithmeticCompression::scaleCounts(const std::array<unsigned long, 256> &counts)
{
    std::array<cch::byte, 256> scaled;

    unsigned long maxCount = *std::max(counts.begin(), counts.end());
    unsigned int total = 1;
    unsigned long scale = maxCount / 256 + 1;

    for (size_t i = 0; i < counts.size(); ++i)
    {
        scaled[i] = static_cast<cch::byte>(counts[i] / scale);

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

/// Implemented
void cch::compression::ArithmeticCompression::buildTotals(const std::array<cch::byte, 256> &scaled)
{
    totals[0] = 0;

    for (size_t i = 0; i < END_OF_STREAM; ++i)
    {
        totals[i + 1] = totals[i] + scaled[i];
    }

    totals[END_OF_STREAM + 1] = totals[END_OF_STREAM] + 1;
}

std::vector<cch::byte> cch::compression::ArithmeticCompression::compress(std::span<cch::byte> data)
{
    buildModel(data);
    initializeArithmeticEncoder();

    for (size_t i = 0; i < data.size() / sizeof(int) + (data.size() % sizeof(int) == 0 ? 0 : 1); ++i)
    {
        int num = *reinterpret_cast<int*>(data.data() + i * sizeof(int));
        auto sym = intToSymbol(num);
        encodeSymbol(sym);
    }

    auto eof = intToSymbol(END_OF_STREAM);
    encodeSymbol(eof);
    flushEncoder();

    out.write(0, 16);
    //out.write()
}

/// Implemented
void cch::compression::ArithmeticCompression::buildModel(std::span<cch::byte> data)
{
    auto counts = countBytes(data);
    auto scaled = scaleCounts(counts);
    buildTotals(scaled);
}

/// Implemented
void cch::compression::ArithmeticCompression::initializeArithmeticEncoder()
{
    low = 0;
    high = 0xFFFF;
    underflowBits = 0;
}

/// Implemented
void cch::compression::ArithmeticCompression::flushEncoder()
{
    out.write(low & 0x4000);
    ++underflowBits;

    while (underflowBits-- > 0)
    {
        out.write(~low & 0x4000);
    }
}

/// Implemented
cch::compression::ArithmeticCompression::Symbol cch::compression::ArithmeticCompression::intToSymbol(int num)
{
    Symbol s;

    s.scale = totals[END_OF_STREAM + 1];
    s.low_count = totals[num];
    s.high_count = totals[num + 1];

    return s;
}

/// Implemented
void cch::compression::ArithmeticCompression::getSymbolScale(cch::compression::ArithmeticCompression::Symbol &s)
{
    s.scale = totals[END_OF_STREAM + 1];
}

int cch::compression::ArithmeticCompression::symbolToInt(int count, cch::compression::ArithmeticCompression::Symbol &s)
{
    int c = 0;

    for (c = END_OF_STREAM; count < totals[c]; --c);

    s.high_count = totals[c + 1];
    s.low_count = totals[c];

    return c;
}

/// Implemented
void cch::compression::ArithmeticCompression::encodeSymbol(const cch::compression::ArithmeticCompression::Symbol &s)
{
    long range = static_cast<long>(high - low) + 1;

    high = low + static_cast<unsigned short>((range * s.high_count) / (s.scale - 1));
    low += static_cast<unsigned short>((range * s.low_count)/(s.scale));

    while (true)
    {
        if ((high & 0x8000) == (low & 0x8000))
        {
            out.write(high & 0x8000);

            while (underflowBits > 0)
            {
                out.write(~high & 0x8000);
                --underflowBits;
            }
        }
        else if ((low & 0x4000) && !(high & 0x4000))
        {
            ++underflowBits;
            low &= 0x3fff;
            high |= 0x4000;
        }
        else
        {
            return;
        }

        low <<= 1;
        high <<= 1;
        high |= 1;
    }
}

