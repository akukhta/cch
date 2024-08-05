#pragma once
#include <vector>

class Utilities
{
public:
    Utilities() = delete;

    static void addElementPerByte(auto const& value, std::vector<char> &vec)
    {
        auto valueBytePtr = reinterpret_cast<char const*>(&value);

        for (size_t i = 0; i < sizeof(value); ++i)
        {
            vec.push_back(valueBytePtr[i]);
        }
    }
};