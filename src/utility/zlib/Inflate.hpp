
#ifndef CHESTER_UTILITY_ZLIB_INFLATE_HPP
#define CHESTER_UTILITY_ZLIB_INFLATE_HPP

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
			class Inflate
			{
			private:
				z_stream stream;
				bool finished;

			public:
				/**  */
				~Inflate()
				{
					::inflateEnd(&stream);
				}
				/**  */
				Inflate() : finished(false)
				{
					stream.zalloc = Z_NULL;
					stream.zfree = Z_NULL;
					stream.opaque = Z_NULL;

					stream.avail_in = 0;
					stream.next_in = Z_NULL;

					const int ret = ::inflateInit(&stream);
					debug_assert(ret == Z_OK);
				}
				/**  */
				Inflate(const Inflate & inflate) = delete;
				/**  */
				Inflate(Inflate && inflate) = delete;

				/**  */
				Inflate & operator = (const Inflate & inflate) = delete;
				/**  */
				Inflate & operator = (Inflate && inflate) = delete;

			public:
				/**  */
				bool isFinished() const
				{
					return finished;
				}

				/**  */
				template <std::size_t N>
				void decompress(const uint8_t (& bytes)[N])
				{
					this->decompress(bytes, N);
				}
				/**  */
				void decompress(const uint8_t *const bytes, const std::size_t count)
				{
					debug_assert(!finished);

					stream.avail_in = count;
					stream.next_in = const_cast<uint8_t *>(bytes);
				}
				/**  */
				template <std::size_t N>
				std::size_t extract(uint8_t (&bytes)[N])
				{
					return this->extract(bytes, N);
				}
				/**  */
				std::size_t extract(uint8_t *const bytes, const std::size_t count)
				{
					stream.avail_out = count;
					stream.next_out = bytes;

					const int ret = ::inflate(&stream, Z_NO_FLUSH);
					debug_assert(ret != Z_NEED_DICT);
					debug_assert(ret != Z_DATA_ERROR);
					debug_assert(ret != Z_MEM_ERROR); // runtime
					debug_assert(ret != Z_STREAM_ERROR); // runtime

					finished = ret == Z_STREAM_END;

					return count - stream.avail_out;
				}
			};
		}
	}
}

#endif /* CHESTER_UTILITY_ZLIB_INFLATE_HPP */
