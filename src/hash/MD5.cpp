#include "../include/hash/MD5.h"
#include <bit>
#include <vector>
#include <algorithm>
#include <iterator>
#include <assert.h>

using namespace cch::hash;

std::array<unsigned char, 64> const constinit MD5::indexTable = MD5::generateIndexTable();

std::array<std::uint_fast32_t, 64> const constinit MD5::K =
{
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

std::array<unsigned char, 64> const constinit MD5::r =
{
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

std::uint_fast32_t MD5::combine(std::uint_fast32_t A, std::uint_fast32_t B, std::uint_fast32_t C, std::uint_fast32_t D, std::uint32_t val, unsigned char i)
{
	std::uint_fast32_t res = f(B, C, D, i) + A + K[i] + val;
	res = std::rotl(res, r[i]);
	res += B;

	return res;
}

MD5Hash MD5::hash(std::vector<cch::byte> data)
{
	MD5Hash hashState;
	hash(data, hashState, data.size());

	return hashState;
}

MD5Hash MD5::hashChunk(std::vector<cch::byte> data)
{
	hash(data, currentHashState, inputDataSize);

	return currentHashState;
}

MD5Hash const & MD5::getHash() const
{
	return currentHashState;
}

void MD5::hash(std::vector<cch::byte> data, MD5Hash &hashState, std::uint64_t inputDataSize)
{
	for (size_t i = 0; i < (data.size() * CHAR_BIT) / 512; ++i)
	{
		_hashChunk({data.begin(), data.end()}, i, hashState);
	}

	if ((data.size() * CHAR_BIT) % 512)
	{
		inputDataSize *= CHAR_BIT;

		std::vector<cch::byte> remainingBytes;
		size_t chunkStartIdx = data.size() - data.size() % 64;
		remainingBytes.reserve(data.size() - chunkStartIdx);

		std::copy(data.begin() + chunkStartIdx, data.end(), std::back_inserter(remainingBytes));

		remainingBytes.push_back(0x80);

		while (remainingBytes.size() * CHAR_BIT % 512 != 448)
		{
			remainingBytes.push_back(0x00);
		}

		char const* dataSizePtr = reinterpret_cast<char const *>(&inputDataSize);
		remainingBytes.insert(remainingBytes.end(), dataSizePtr, dataSizePtr + sizeof(inputDataSize));

		for (size_t i = 0; i < (remainingBytes.size() * CHAR_BIT) / 512; ++i)
		{
			_hashChunk({remainingBytes.begin(), remainingBytes.end()}, i, hashState);
		}
	}
}

std::uint_fast32_t MD5::f(std::uint_fast32_t B, std::uint_fast32_t C, std::uint_fast32_t D, unsigned char i)
{
	if (i >= 0 && i <= 15)
	{
		return (B & C) | ((~B) & D);
	}

	if (i >= 16 && i <= 31)
	{
		return (B & D) | ((~D) & C);
	}

	if (i >= 32 && i <= 47)
	{
		return B ^ C ^ D;
	}

	if (i >= 48 && i <= 63)
	{
		return C ^ (B | (~D));
	}

	return 0x00000000;
}

MD5Hash MD5::calculateHash(std::span<std::uint_fast32_t> data, MD5Hash& hashState)
{
	assert(data.size() * sizeof(std::uint_fast32_t) * CHAR_BIT == 512);

	std::uint_fast32_t A = hashState.A;
	std::uint_fast32_t B = hashState.B;
	std::uint_fast32_t C = hashState.C;
	std::uint_fast32_t D = hashState.D;

	for (size_t i = 0; i < 64; ++i) {
		std::uint_fast32_t Atmp = D;
		std::uint_fast32_t Btmp = combine(A, B, C, D, data[indexTable[i]], i);
		std::uint_fast32_t Ctmp = B;
		std::uint_fast32_t Dtmp = C;

		A = Atmp;
		B = Btmp;
		C = Ctmp;
		D = Dtmp;
	}

	return MD5Hash{ A, B, C, D};
}

void MD5::_hashChunk(std::span<cch::byte> data, size_t chunkIdx, MD5Hash& hashState)
{
	std::vector<cch::byte> block;
	block.reserve(64);
	std::copy(data.begin() + chunkIdx * 64, data.begin() + (chunkIdx + 1) * 64, std::back_inserter(block));

	assert(block.size() == 64);
	std::uint_fast32_t* int32Ptr = reinterpret_cast<std::uint_fast32_t*>(block.data());

	hashState += calculateHash({ int32Ptr, block.size() / sizeof(std::uint_fast32_t)}, hashState);
}