#pragma once
#include <iterator>

struct Input{};
struct Output{};

template <typename, typename>
class bitstream;

/// Read Only bitstream
/// @tparam InputIterator Iterators point to the begin and to the end of a stream
template <typename InputIterator>
    requires std::input_iterator<InputIterator>
class bitstream<InputIterator, Input>
{
public:
    bitstream(InputIterator first, InputIterator last)
        : currentIt(first), last(last) {}

    bool read()
    {
        // If the current byte poiner is null, we should read the next value
        if (bytePtr == nullptr)
        {
            // the end of a strem has reached
            if (currentIt == last)
            {
                throw std::runtime_error("bitstream out of bounds");
            }

            bytePtr = reinterpret_cast<unsigned char const *>(&(*currentIt));
            ++currentIt;

            // Reset the byte index and mask
            currentIndex = 0;
            mask = 0x80;
        }

        // calculate the return value and shift the mask
        bool rv = bytePtr[currentIndex] & mask;
        mask >>= 1;

        if (mask == 0x00)
        {
        // The entire byte has been read, move to the next byte or set it to null
        // to read the next value from the iterator
            if (currentIndex + 1 < bytesPerValue)
            {
                ++currentIndex;
                mask = 0x80;
            }
            else
            {
                bytePtr = nullptr;
            }
        }

        return rv;
    }

private:
    InputIterator currentIt;
    InputIterator last;

    unsigned char const* bytePtr = nullptr;
    size_t currentIndex;

    unsigned char mask = 0x8A;
    static size_t const inline bytesPerValue = sizeof(typename std::iterator_traits<InputIterator>::value_type);
};

template <typename InputIterator>
bitstream(InputIterator, InputIterator) -> bitstream<InputIterator, Input>;

template <typename ValueType>
    requires (!std::input_iterator<ValueType>)
class bitstream<ValueType, Input>
{
public:
    bitstream(ValueType const &value)
        : bytePtr(reinterpret_cast<unsigned char const*>(&value))
    {};

    bool read() {

        if (mask == 0x00)
        {
            mask = 0x80;
            ++currentIndex;

            if (currentIndex > sizeof(ValueType))
            {
                throw std::runtime_error("bitstream out of bounds");
            }
        }

        bool rv = bytePtr[currentIndex] & mask;
        mask >>= 1;

        return rv;
    }

private:
    unsigned char const* bytePtr;
    unsigned char mask = 0x80;
    size_t currentIndex = 0;
};

template <typename ValueType>
bitstream(ValueType const&) -> bitstream<ValueType, Input>;

template <typename ValueType>
    requires (!std::input_iterator<ValueType>)
class bitstream<ValueType, Output>
{
public:
    bitstream(ValueType &value)
        : bytePtr(reinterpret_cast<unsigned char*>(&value))
    {};

    void write(bool value)
    {
        if (mask == 0x00)
        {
            mask = 0x80;
            ++currentIndex;

            if (currentIndex >= sizeof(ValueType))
            {
                throw std::runtime_error("bitstream out of bounds");
            }
        }

        if (value)
        {
            bytePtr[currentIndex] |= (0xFF & mask);
        }

        mask >>= 1;
    }

private:
    unsigned char* bytePtr;
    unsigned char mask = 0x80;
    size_t currentIndex = 0;
};

template <typename ValueType>
bitstream(ValueType const&) -> bitstream<ValueType, Input>;


template <>
class bitstream<char, Output>
{
public:
    bitstream() = default;

    void write(bool v)
    {
        if (mask == 0x00)
        {
            currentByte = 0x00;
            mask = 0x80;
        }

        if (v)
        {
            currentByte |= ((v == true ? 0xFF : 0x00) & mask);
        }

        mask >>= 1;
    }

    char get()
    {
        return currentByte;
    }

private:
    unsigned char currentByte = 0x00;
    unsigned char mask = 0x80;
};

//template <>
//bitstream() -> bitstream<char, Output>;