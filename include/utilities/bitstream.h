#pragma once
#include <iterator>
#include <stdexcept>
#include <cassert>

template <typename>
class ibitstream;

/// InputIterator input bitstream
/// @tparam InputIterator Input Iterators point to the begin and to the end of a stream
template <typename InputIterator>
    requires std::input_iterator<InputIterator>
class ibitstream<InputIterator>
{
public:
    /// InputIterator bitstream Constructor
    /// \param first input iterator to the first element read bytes from
    /// \param last  input iterator to the last element read bytes from
    ibitstream(InputIterator first, InputIterator last) noexcept
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
            currentStreamPos += 1;
            // Reset the byte index and mask
            currentIndex = 0;
            mask = 0x80;
        }

        // calculate the return value and shift the mask
        bool const rv = bytePtr[currentIndex] & mask;
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

    /// Extract one bit from the stream
    /// @param ibitstream bitstream object
    /// @param value return value
    friend auto& operator>>(ibitstream<InputIterator> &ibitstream, bool &value)
    {
        value = ibitstream.read();
        return ibitstream;
    }

    bool eof() const noexcept
    {
        return bytePtr == nullptr && currentIt == last;
    }

    bool isLastByte() const noexcept
    {
        return currentIt  == last;
    }

    auto tellg() const noexcept
    {
        return currentStreamPos;
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
    std::streampos currentStreamPos = 0;
    unsigned char mask = 0x80;
};

/// type deduction guide for InputIterator input bitstream
template <typename InputIterator>
ibitstream(InputIterator, InputIterator) -> ibitstream<InputIterator>;

/// Scalar Value input bitstream
/// \tparam ValueType Type of the value we read bits from
template <typename ValueType>
    requires (!std::input_iterator<ValueType>)
class ibitstream<ValueType>
{
public:
    /// Scalar input bitstream
    /// \param value Reference to the value to read bits from
    explicit ibitstream(ValueType const &value) noexcept
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
        bool const rv = bytePtr[currentIndex] & mask;
        mask >>= 1;

        return rv;
    }

    /// Extract one bit from the stream
    /// @param ibitstream bitstream object
    /// @param value return value
    friend auto& operator>>(ibitstream<ValueType> &ibitstream, bool &value)
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
ibitstream(ValueType const&) -> ibitstream<ValueType>;

template <typename>
class obitstream;

/// Output iterator output bitstream
/// \tparam OutputIterator Output Iterators point to the begin and to the end of a stream
template <typename OutputIterator>
    requires std::output_iterator<OutputIterator, typename OutputIterator::value_type>
class obitstream<OutputIterator>
{
public:
    /// OutputIterator bitstream Constructor
    /// \param first Output iterator to the first element write bytes to
    /// \param last  Output iterator to the last element write bytes to
    obitstream(OutputIterator first, OutputIterator last)
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
    void write(bool const value)
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

    /// Insert one bit to the stream
    /// @param obitstream bitstream object
    /// @param value bit to insert
    friend auto& operator<<(obitstream<OutputIterator> &obitstream, bool const value)
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
obitstream(OutputIterator first, OutputIterator last) -> obitstream<OutputIterator>;


/// output bitstream with internal buffer
/// \tparam OutputIterator Output Iterators point to the begin and to the end of a stream
template <>
class obitstream<std::vector<unsigned char>>
{
public:
    obitstream() noexcept = default;

    /// Write single bit to the stream
    /// \param value bit to write to the stream true/false (1/0)
    void write(bool const value)
    {
        // If the current byte pointer is null, we should read the next value
        if (bytePtr == nullptr)
        {
            buffer.push_back(0x00);

            bytePtr = &buffer.back();
            // Reset the byte index and mask
            mask = 0x80;
            bitsWrittenToCurrentByte = 0;
        }

        // write bit and shift the mask
        if (value)
        {
            *bytePtr |= mask;
            ++bitsWrittenToCurrentByte;
        }
        else
        {
            *bytePtr &= ~mask;
            ++bitsWrittenToCurrentByte;
        }

        mask >>= 1;

        if (mask == 0x00)
        {
            bytePtr = nullptr;
        }
    }

    std::vector<unsigned char>& getBuffer() noexcept
    {
        return buffer;
    }

    std::vector<unsigned char> extractBuffer()
    {
        auto rv = std::move(buffer);
        buffer = std::vector<unsigned char>{};

        return rv;
    }

    unsigned char getBitsWrittenToCurrentByte() const noexcept
    {
        return bitsWrittenToCurrentByte;
    }

    /// Insert one bit to the stream
    /// @param obitstream bitstream object
    /// @param value bit to insert
    friend auto& operator<<(obitstream<std::vector<unsigned char>> &obitstream, bool const value)
    {
        obitstream.write(value);
        return obitstream;
    }
private:
    std::vector<unsigned char> buffer;
    unsigned char* bytePtr = nullptr;
    unsigned char mask = 0x80;
    unsigned char bitsWrittenToCurrentByte = 0;
};

/// Scalar Value output bitstream
/// \tparam ValueType Type of the value to write bits to
template <typename ValueType>
    requires (!std::input_iterator<ValueType>)
class obitstream<ValueType>
{
public:
    /// Scalar output bitstream
    /// \param value Reference to the value to write bits to
    explicit obitstream(ValueType &value) noexcept
        : bytePtr(reinterpret_cast<unsigned char*>(&value))
    {};

    /// Write single bit to the stream
    /// \param value bit to write to the stream true/false (1/0)
    void write(bool const value)
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

    /// Insert one bit to the stream
    /// @param obitstream bitstream object
    /// @param value bit to insert
    friend auto& operator<<(obitstream<ValueType> &obitstream, bool const value)
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

/// type deduction guide for OutputIterator bitstream
template <typename ValueType>
obitstream(ValueType) -> obitstream<ValueType>;