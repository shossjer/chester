
#ifndef CHESTER_UTILITY_CRYPTO_SHA256_HPP
#define CHESTER_UTILITY_CRYPTO_SHA256_HPP

#include <cstdint>

namespace chester
{
	namespace utility
	{
		namespace crypto
		{
			class Sha256
			{
			private:
				/** A Quad is a 64-bit unsigned integer. */
				union Quad
				{
					/** Common sizes for a Quad. */
					enum
					{
						NUMBER_OF_BYTES = 8,
						NUMBER_OF_WORDS = 2
					};

					/** A Quad can be viewed as an array of bytes. */
					uint8_t bytes[Quad::NUMBER_OF_BYTES];
					/** A Quad can be viewed as an array of words. */
					uint32_t words[Quad::NUMBER_OF_WORDS];
					/** A Quad can be viewed as a regular quad. */
					uint64_t quad;

					/** Constructor of two words. */
					Quad(const uint_fast32_t hiword, const uint_fast32_t loword)
					{
						words[1] = hiword;
						words[0] = loword;
					}
				};

				/** A Block contains 512-bits of message. */
				union Block
				{
					/** Common sizes for a Block. */
					enum
					{
						NUMBER_OF_BITS  = 512,
						NUMBER_OF_BYTES = 512 /  8,
						NUMBER_OF_WORDS = 512 / 32,
						NUMBER_OF_QUADS = 512 / 64
					};

					/** A Block can be viewed as an array of bytes. */
					uint8_t bytes[Block::NUMBER_OF_BYTES];
					/** A Block can be viewed as an array of words. */
					uint32_t words[Block::NUMBER_OF_WORDS];
					/** A Block can be viewed as an array of quads. */
					uint64_t quads[Block::NUMBER_OF_QUADS];
				};
				/** A Chunk contains the expansion of a Block. */
				union Chunk
				{
					/** Common sizes for a Chunk. */
					enum
					{
						NUMBER_OF_BYTES = 64 * 4,
						NUMBER_OF_WORDS = 64
					};

					/** A Chunk can be viewed as an array of bytes. */
					uint8_t bytes[Block::NUMBER_OF_BYTES];
					/** A Chunk can be viewed as an array of words. */
					uint32_t words[Block::NUMBER_OF_WORDS];
					/** A Chunk can be viewed as a block. */
					struct
					{
						/** Padding so the block is aligned correctly. */
						uint8_t padding[Chunk::NUMBER_OF_BYTES - Block::NUMBER_OF_BYTES];
						/** The Block. */
						Block block;
					} piece;
				};
				/** A Message contains 8 words of message. */
				union Message
				{
					/** Common sizes for a Message. */
					enum
					{
						NUMBER_OF_WORDS = 8
					};

					/** A Message can be viewed as an array of words. */
					uint32_t words[Message::NUMBER_OF_WORDS];
				};

			private:
				/** A buffer for the message. */
				Message message;
				/** A buffer for the chunk. */
				Chunk chunk;

				/** The number of blocks. */
				unsigned int n;
				/** The index of the byte. */
				unsigned int b;

			public:
				Sha256() :
					n(0),
					b(Block::NUMBER_OF_BYTES - 1)
				{
					message.words[0] = 0x6a09e667; // a
					message.words[1] = 0xbb67ae85; // b
					message.words[2] = 0x3c6ef372; // c
					message.words[3] = 0xa54ff53a; // d
					message.words[4] = 0x510e527f; // e
					message.words[5] = 0x9b05688c; // f
					message.words[6] = 0x1f83d9ab; // g
					message.words[7] = 0x5be0cd19; // h
				}

			public:
				void update(const uint8_t *const bytes, const std::size_t count)
				{
					for (std::size_t i = 0; i < count; i++)
					{
						// read a byte
						chunk.piece.block.bytes[b] = bytes[i];
						// if the next byte index is nonexisting...
						if (--b >= Block::NUMBER_OF_BYTES)
						{
							// reset the index
							b = Block::NUMBER_OF_BYTES - 1;
							// expand chunk
							expand(chunk);
							// mix it with the message
							mix(chunk, message);
							// increase the number of blocks
							++n;
						}
					}
				}
				void finalize(uint8_t *const bytes)
				{
					/** The length of the message (in bits). */
					const uint_fast64_t length = n * Block::NUMBER_OF_BITS + ((Block::NUMBER_OF_BYTES - 1) - b) * 8;
					// append 1 to the message
					chunk.piece.block.bytes[b] = 0x80;
					// if we can't fit the length right know...
					if (b < 8)
					{
						// fill with zeros
						for (unsigned int i = 0; i < b; ++i)
							chunk.piece.block.bytes[i] = 0x00;
						// expand chunk
						expand(chunk);
						// mix it with the message
						mix(chunk, message);
						// fill with zeros
						for (unsigned int i = 2; i < Block::NUMBER_OF_WORDS; ++i)
							chunk.piece.block.words[i] = 0x00000000;
					}
					else
					{
						// fill with zeros
						for (unsigned int i = 8; i < b; ++i)
							chunk.piece.block.bytes[i] = 0x00;
					}
					// append length
					chunk.piece.block.quads[0] = length;
					// expand chunk
					expand(chunk);
					// mix it with the message
					mix(chunk, message);

					for (int i = 0; i < 8; i++)
					{
						// switch endianess
						auto x = message.words[i];
						x = (x >> 16) | (x << 16);
						x = ((x >> 8) & 0x00ff00ff) | ((x << 8) & 0xff00ff00);
						reinterpret_cast<uint32_t *>(bytes)[i] = x;
					}
					// the above loop (as well as the class structure)
					// is  a quick  fix  and was  not  present in  the
					// original  code, written  by me  some years  ago
					// when I  actually understood what the  **** this
					// code does.

					// * why is it necessary to switch endianess?
					// * what was I thinking?
				}
			private:
				/** Rotates a word a fix number of bits. */
				template <unsigned int N>
				uint32_t rotate(const uint_fast32_t word)
				{
					// Rotate the word by shifting a quad.
					Quad quad(word, word);

					quad.quad >>= N;

					return quad.words[0];
				}
				/** Shifts a word a fix number of bits. */
				template <unsigned int N>
				uint32_t shift(const uint_fast32_t word)
				{
					return word >> N;
				}

				/** Expands a block into a whole chunk. */
				void expand(Chunk &chunk)
				{
					// for all words after the block...
					for (unsigned int i = (Chunk::NUMBER_OF_WORDS - Block::NUMBER_OF_WORDS) - 1; i < Chunk::NUMBER_OF_WORDS - Block::NUMBER_OF_WORDS; --i)
					{
						/** Some local variables. */
						const uint32_t word_2 = chunk.words[i + 2];
						const uint32_t word_7 = chunk.words[i + 7];
						const uint32_t word_15 = chunk.words[i + 15];
						const uint32_t word_16 = chunk.words[i + 16];
						// calculate the next word
						chunk.words[i] = (rotate<17>(word_2) ^ rotate<19>(word_2) ^ shift<10>(word_2)) +
							word_7 +
							(rotate<7>(word_15) ^ rotate<18>(word_15) ^ shift<3>(word_15)) +
							word_16;
					}
				}
				/** Mixes a chunk together with a message. */
				void mix(const Chunk &chunk, Message &message)
				{
					/** Magic numbers. (copied from http://en.wikipedia.org/wiki/Sha256, 2013-05-14 18:51) */
					const uint32_t Ks[64] = {
						0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
						0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
						0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
						0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
						0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
						0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
						0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
						0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
						0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
						0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
						0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
						0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
						0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
						0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
						0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
						0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
					};
					/** A buffer for the message. */
					Message abcdefgh = message;
					// for 64 iterations...
					for (unsigned int i = 0; i < 64; ++i)
					{
						/** Some local variables. */
						const uint32_t a = abcdefgh.words[0];
						const uint32_t b = abcdefgh.words[1];
						const uint32_t c = abcdefgh.words[2];
						const uint32_t d = abcdefgh.words[3];
						const uint32_t e = abcdefgh.words[4];
						const uint32_t f = abcdefgh.words[5];
						const uint32_t g = abcdefgh.words[6];
						const uint32_t h = abcdefgh.words[7];
						/** A temporary variable. */
						const uint32_t t_1 = h +
							(rotate<6>(e) ^ rotate<11>(e) ^ rotate<25>(e)) +
							((e & f) ^ ((~e) & g)) +
							Ks[i] +
							chunk.words[64 - 1 - i];
						/** A temporary variable. */
						const uint32_t t_2 = (rotate<2>(a) ^ rotate<13>(a) ^ rotate<22>(a)) +
							((a & b) ^ (a & c) ^ (b & c));
						// update the message for the next iteration
						abcdefgh.words[7] = g;
						abcdefgh.words[6] = f;
						abcdefgh.words[5] = e;
						abcdefgh.words[4] = d + t_1;
						abcdefgh.words[3] = c;
						abcdefgh.words[2] = b;
						abcdefgh.words[1] = a;
						abcdefgh.words[0] = t_1 + t_2;
					}
					// apply the block of message to the original message
					for (unsigned int i = 0; i < Message::NUMBER_OF_WORDS; ++i)
						message.words[i] += abcdefgh.words[i];
				}
			};
		}
	}
}

#endif /* CHESTER_UTILITY_CRYPTO_SHA256_HPP */
