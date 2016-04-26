
#ifndef CHESTER_COMMON_CODE_HPP
#define CHESTER_COMMON_CODE_HPP

#include <utility/debug.hpp>
#include <utility/serialize.hpp>
#include <utility/structure.hpp>

#include <iomanip>
#include <istream>
#include <ostream>
#include <tuple>

namespace chester
{
	namespace common
	{
		class Code
		{
		public:
			using this_type = Code;
			using iterator = uint8_t *;
			using const_iterator = const uint8_t *;

		private:
			/**
			 * warning warning warning warning warning warning warning
			 *             this is not standard compliant
			 * warning warning warning warning warning warning warning
			 */
			union
			{
				uint64_t quads[4];
				uint8_t bytes[32];
			};

		public:
			iterator data()
			{
				return bytes;
			}
			const_iterator data() const
			{
				return bytes;
			}

		public:
			static Code null()
			{
				Code code;
				code.quads[0] = 0;
				code.quads[1] = 0;
				code.quads[2] = 0;
				code.quads[3] = 0;
				return code;
			}

		public:
			friend bool operator == (const this_type & code1, const this_type & code2)
			{
				return
					std::tie(code1.quads[0], code1.quads[1],
					         code1.quads[2], code1.quads[3]) ==
					std::tie(code2.quads[0], code2.quads[1],
					         code2.quads[2], code2.quads[3]);
			}
			friend bool operator < (const this_type & code1, const this_type & code2)
			{
				return
					std::tie(code1.quads[0], code1.quads[1],
					         code1.quads[2], code1.quads[3]) <
					std::tie(code2.quads[0], code2.quads[1],
					         code2.quads[2], code2.quads[3]);
			}

			friend std::ostream & operator << (std::ostream & stream, const this_type & code)
			{
				for (auto && byte : code.bytes)
				{
					stream << std::hex
					       << std::setw(2)
					       << std::setfill('0')
					       << int(byte);
				}
				return stream;
			}
			friend std::istream & operator >> (std::istream & stream, this_type & code)
			{
				for (auto && byte : code.bytes)
				{
					// stream >> std::hex
					//        >> byte;
					// debug_assert(stream);
					auto c = stream.get();
					     if (c >= '0' && c <= '9') c +=  0 - '0';
					else if (c >= 'A' && c <= 'F') c += 10 - 'A';
					else if (c >= 'a' && c <= 'f') c += 10 - 'a';
					auto d = stream.get();
					     if (d >= '0' && d <= '9') d +=  0 - '0';
					else if (d >= 'A' && d <= 'F') d += 10 - 'A';
					else if (d >= 'a' && d <= 'f') d += 10 - 'a';
					byte = (c << 4) | d;
				}
				return stream;
			}

			template <typename S>
			friend void serialize(S & s, const this_type & code)
			{
				using utility::serialize;
				serialize(s, code.quads[0]);
				serialize(s, code.quads[1]);
				serialize(s, code.quads[2]);
				serialize(s, code.quads[3]);
			}
			template <typename S>
			friend void structure(S & s, this_type & code)
			{
				using utility::structure;
				structure(s, code.quads[0]);
				structure(s, code.quads[1]);
				structure(s, code.quads[2]);
				structure(s, code.quads[3]);
			}
		};
	}
}

#endif /* CHESTER_COMMON_CODE_HPP */
