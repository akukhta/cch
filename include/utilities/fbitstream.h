#pragma once
#include <string>
#include <fstream>
#include <bitset>

class fbitstream
{
public:
    explicit fbitstream(std::string const& filePath);

    bool is_open() const noexcept;
    bool read();
    void write(bool value);
    void close();

private:
    std::fstream byteIO;
    unsigned char currentByte;
    unsigned char mask{0x80};
};