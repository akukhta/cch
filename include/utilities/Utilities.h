#pragma once
#include <vector>

class Utilities
{
public:
    Utilities() = delete;

    template <class VectorValueType>
    static void addElementPerByte(auto const& value, std::vector<VectorValueType> &vec)
        requires (std::is_same_v<VectorValueType, char> || std::is_same_v<VectorValueType, unsigned char>)
    {
        auto valueBytePtr = reinterpret_cast<char const*>(&value);

        for (size_t i = 0; i < sizeof(value); ++i)
        {
            vec.push_back(valueBytePtr[i]);
        }
    }
};