#include "fbitstream.h"

bool fbitstream::is_open() const noexcept
{
    return byteIO.is_open();
}

bool fbitstream::read()
{
   if (mask == 0x00)
   {
       byteIO >> currentByte;
       mask = 0x80;
   }

    bool const rv = (currentByte & mask);
    mask >>= 1;

    return rv;
}

void fbitstream::write(bool value)
{
    currentByte |= mask;
    mask >>= 1;

    if (mask == 0x00)
    {
        byteIO << currentByte;
        currentByte = 0x00;
        mask = 0x80;
    }
}

void fbitstream::close()
{
    byteIO.close();
}
