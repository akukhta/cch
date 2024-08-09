#pragma once
#include <iterator>
#include <stdexcept>
#include <cassert>

struct Input{};
struct Output{};

template <typename, typename>
class bitstream;

/// InputIterator input bitstream
/// @tparam InputIterator Input Iterators point to the begin and to the end of a stream
template <typename InputIterator>
    requires std::input_iterator<InputIterator>
class bitstream<InputIterator, Input>
{
public:
    /// InputIterator bitstream Constructor
    /// \param first input iterator to the first element read bytes from
    /// \param last  input iterator to the last element read bytes from
    bitstream(InputIterator first, InputIterator last) noexcept
        : currentIt(first), last(last) {}

    /// Read single bit from a stream
    /// \return true/false (1/0)
    bool read()
    {
        // If the current byte pointer is null, we should read the next value
        if (bytePtr == nullptr)
        {
            // the end of a stream has reached
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
            if (currentIndex + 1 < sizeof(typename std::iterator_traits<InputIterator>::value_type))
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

    friend auto& operator>>(bitstream<InputIterator, Input> &ibitstream, bool &value)
    {
        value = ibitstream.read();
        return ibitstream;
    }

private:
    /// Beginning of the stream
    InputIterator currentIt;
    /// End of the stream
    InputIterator last;

    /// Pointer to the current byte to read bits from
    unsigned char const* bytePtr = nullptr;
    /// Current byte index
    size_t currentIndex = 0;
    unsigned char mask = 0x80;
};

/// type deduction guide for InputIterator input bitstream
template <typename InputIterator>
bitstream(InputIterator, InputIterator) -> bitstream<InputIterator, Input>;

/// Scalar Value input bitstream
/// \tparam ValueType Type of the value we read bits from
template <typename ValueType>
    requires (!std::input_iterator<ValueType>)
class bitstream<ValueType, Input>
{
public:
    /// Scalar input bitstream
    /// \param value Reference to the value to read bits from
    explicit bitstream(ValueType const &value) noexcept
        : bytePtr(reinterpret_cast<unsigned char const*>(&value))
    {};

    /// Read single bit from a stream
    /// \return current bit from true/false (1/0)
    bool read()
    {
        // Check if the entire 8 bits have been read
        if (mask == 0x00)
        {
            // restore the bit mask and increase the byte index
            mask = 0x80;
            ++currentIndex;

            // Check there are still bits to read from the value
            if (currentIndex >= sizeof(ValueType))
            {
                throw std::runtime_error("bitstream out of bounds");
            }
        }

        // Read current bit and shift the mask
        bool rv = bytePtr[currentIndex] & mask;
        mask >>= 1;

        return rv;
    }

    friend auto& operator>>(bitstream<ValueType, Input> &ibitstream, bool &value)
    {
        value = ibitstream.read();
        return ibitstream;
    }

private:
    /// Pointer to the current byte to read bits from
    unsigned char const* bytePtr;
    unsigned char mask = 0x80;
    /// Current byte index
    size_t currentIndex = 0;
};

/// Type deduction guide for Scalar input bitstream
template <typename ValueType>
bitstream(ValueType const&) -> bitstream<ValueType, Input>;

/// Output iterator output bitstream
/// \tparam OutputIterator Output Iterators point to the begin and to the end of a stream
template <typename OutputIterator>
    requires std::output_iterator<OutputIterator, typename OutputIterator::value_type>
class bitstream<OutputIterator, Output>
{
public:
    /// OutputIterator bitstream Constructor
    /// \param first Output iterator to the first element write bytes to
    /// \param last  Output iterator to the last element write bytes to
    bitstream(OutputIterator first, OutputIterator last)
        : currentIt(first), last(last)
    {
        assert(currentIt != last);

        if (currentIt != last)
        {
            bytePtr = reinterpret_cast<unsigned char *>(&(*currentIt));

            // Reset the byte index and mask
            currentIndex = 0;
        }
    };

    /// Write single bit to the stream
    /// \param value bit to write to the stream true/false (1/0)
    void write(bool value)
    {
        // If the current byte pointer is null, we should read the next value
        if (bytePtr == nullptr)
        {
            // the end of a stream has reached
            if ((++currentIt) == last)
            {
                throw std::runtime_error("bitstream out of bounds");
            }

            bytePtr = reinterpret_cast<unsigned char *>(&(*currentIt));

            // Reset the byte index and mask
            currentIndex = 0;
            mask = 0x80;
        }

        // write bit and shift the mask
        if (value)
        {
            bytePtr[currentIndex] |= mask;
        }
        else
        {
            bytePtr[currentIndex] &= ~mask;
        }

        mask >>= 1;

        if (mask == 0x00)
        {
            // The entire byte has been written, move to the next byte or set it to null
            // to obtain the next value from the iterator
            if (currentIndex + 1 < sizeof(typename OutputIterator::value_type))
            {
                ++currentIndex;
                mask = 0x80;
            }
            else
            {
                bytePtr = nullptr;
            }
        }
    }

    friend auto& operator<<(bitstream<OutputIterator, Output> &obitstream, bool value)
    {
        obitstream.write(value);
        return obitstream;
    }

private:
    /// Beginning of the stream
    OutputIterator currentIt;
    /// End of the stream
    OutputIterator last;
    /// Pointer to the current byte to write bits to
    unsigned char* bytePtr = nullptr;
    unsigned char mask = 0x80;
    /// Current byte index
    size_t currentIndex = 0;
};

/// type deduction guide for OutputIterator bitstream
template <typename OutputIterator>
bitstream(OutputIterator first, OutputIterator last) -> bitstream<OutputIterator, Output>;

/// Scalar Value output bitstream
/// \tparam ValueType Type of the value to write bits to
template <typename ValueType>
    requires (!std::input_iterator<ValueType>)
class bitstream<ValueType, Output>
{
public:
    /// Scalar output bitstream
    /// \param value Reference to the value to write bits to
    explicit bitstream(ValueType &value) noexcept
        : bytePtr(reinterpret_cast<unsigned char*>(&value))
    {};

    /// Write single bit to the stream
    /// \param value bit to write to the stream true/false (1/0)
    void write(bool value)
    {
        // Check if the entire 8 bits have been written
        if (mask == 0x00)
        {
            // Reset binary mask and increase byte pointer index
            mask = 0x80;
            ++currentIndex;

            if (currentIndex >= sizeof(ValueType))
            {
                throw std::runtime_error("bitstream out of bounds");
            }
        }

        // Write 1 bit to the stream
        if (value)
        {
            bytePtr[currentIndex] |= mask;
        }
        else
        {
            bytePtr[currentIndex] &= ~mask;
        }

        // Shift the binary mask by 1 bit to right
        mask >>= 1;
    }

    friend auto& operator<<(bitstream<ValueType, Output> &obitstream, bool value)
    {
        obitstream.write(value);
        return obitstream;
    }

private:
    /// Pointer to the current byte to write bits to
    unsigned char* bytePtr;
    unsigned char mask = 0x80;
    /// Current byte index
    size_t currentIndex = 0;
};

template <typename ValueType>
bitstream(ValueType const&) -> bitstream<ValueType, Output>;

template <typename ValueType>
using ibitstream = bitstream<ValueType, Input>;

template <typename ValueType>
using obitstream = bitstream<ValueType, Output>;