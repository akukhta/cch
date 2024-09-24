#pragma once
#include <span>
#include <array>
#include <vector>
#include <sstream>
#include <iomanip>
#include "../config/types.h"

namespace cch::hash
{
	struct MD5Hash
	{
		union
		{
			cch::byte rawHash[16];
			std::uint_fast32_t parts[4];

			struct
			{
				std::uint_fast32_t A;
				std::uint_fast32_t B;
				std::uint_fast32_t C;
				std::uint_fast32_t D;
			};
		};

		std::string toString() const
		{
			std::stringstream ss;

			for (size_t i = 0; i < std::size(rawHash); ++i)
			{
				ss << std::setw(2) << std::setfill('0') << std::hex << (static_cast<int>(rawHash[i]) & 0xFF);
			}

			return ss.str();
		}

		std::span<cch::byte const> const getHash() const
		{
			return std::span<cch::byte const>(rawHash);
		}

		MD5Hash& operator+=(MD5Hash const& other)
		{
			A += other.A;
			B += other.B;
			C += other.C;
			D += other.D;

			return *this;
		}

		explicit MD5Hash(size_t inputDataSize = 0) : A(A0), B(B0), C(C0), D(D0) {}

		MD5Hash(std::uint_fast32_t A, uint_fast32_t B, uint_fast32_t C, uint_fast32_t D) :
			A(A), B(B), C(C), D(D) {}

		static std::uint_fast32_t const inline A0 = 0x67452301;
		static std::uint_fast32_t const inline B0 = 0xefcdab89;
		static std::uint_fast32_t const inline C0 = 0x98badcfe;
		static std::uint_fast32_t const inline D0 = 0x10325476;
	};

	class MD5
	{
	public:
		static MD5Hash hash(std::span<cch::byte> data);

	private:

		// Static Methods and Variables
		static consteval std::array<unsigned char, 64> generateIndexTable()
		{
			std::array<unsigned char, 64> table;

			for (unsigned char i = 0; i < 16; ++i)
			{
				table[i] = i;
			}

			for (unsigned char i = 0; i < 16; ++i)
			{
				table[16 + i] = (5 * i + 1) % 16;
			}

			for (unsigned char i = 0; i < 16; ++i)
			{
				table[32 + i] = (3 * i + 5) % 16;
			}

			for (unsigned char i = 0; i < 16; ++i)
			{
				table[48 + i] = (7 * i) % 16;
			}

			return table;
		}

		static void hash(std::span<cch::byte> data, MD5Hash &hashState, std::uint64_t inputDataSize);

		static std::array<unsigned char, 64> const constinit indexTable;

		static std::array<std::uint_fast32_t, 64> const constinit K;

		static std::array<unsigned char, 64> const constinit r;

		static std::uint_fast32_t combine(std::uint_fast32_t A, std::uint_fast32_t B, std::uint_fast32_t C, std::uint_fast32_t D, std::uint32_t val, unsigned char i);

		static MD5Hash calculateHash(std::span<std::uint_fast32_t> data, MD5Hash& hashState);
		static void _hashChunk(std::span<cch::byte> data, size_t chunkIdx, MD5Hash& hashState);

		static std::uint_fast32_t f(std::uint_fast32_t B, std::uint_fast32_t C, std::uint_fast32_t D, unsigned char i);
	};
}
