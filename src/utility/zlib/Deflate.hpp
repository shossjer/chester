
#ifndef CHESTER_UTILITY_ZLIB_DEFLATE_HPP
#define CHESTER_UTILITY_ZLIB_DEFLATE_HPP

#include <utility/debug.hpp>

#include <zlib.h>

#include <cstdint>
#include <stdexcept>

namespace chester
{
	namespace utility
	{
		namespace zlib
		{
			class Deflate
			{
			private:
				z_stream stream;
				int flush;

			public:
				/**  */
				~Deflate()
				{
					::deflateEnd(&stream);
				}
				/**  */
				Deflate()
				{
					stream.zalloc = Z_NULL;
					stream.zfree = Z_NULL;
					stream.opaque = Z_NULL;

					const int ret = ::deflateInit(&stream, Z_DEFAULT_COMPRESSION);
					debug_assert(ret == Z_OK);
				}
				/**  */
				Deflate(const Deflate & deflate) = delete;
				/**  */
				Deflate(Deflate && deflate) = delete;

				/**  */
				Deflate & operator = (const Deflate & deflate) = delete;
				/**  */
				Deflate & operator = (Deflate && deflate) = delete;

			public:
				/**  */
				template <std::size_t N>
				void compress(const uint8_t (& bytes)[N], bool end = false)
				{
					this->compress(bytes, N, end);
				}
				/**  */
				void compress(const uint8_t *const bytes, const std::size_t count, const bool end = false)
				{
					stream.avail_in = count;
					stream.next_in = const_cast<uint8_t *>(bytes);

					flush = end ? Z_FINISH : Z_NO_FLUSH;
				}
				/**  */
				template <std::size_t N>
				std::size_t extract(uint8_t (& bytes)[N])
				{
					return this->extract(bytes, N);
				}
				/**  */
				std::size_t extract(uint8_t *const bytes, const std::size_t count)
				{
					stream.avail_out = count;
					stream.next_out = bytes;

					const int ret = ::deflate(&stream, flush);
					debug_assert(ret != Z_STREAM_ERROR);

					return count - stream.avail_out;
				}
			};
		}
	}
}

#endif /* CHESTER_UTILITY_ZLIB_DEFLATE_HPP */
