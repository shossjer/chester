
#ifndef CHESTER_COMMON_HEADER_HPP
#define CHESTER_COMMON_HEADER_HPP

#include <utility/serialize.hpp>
#include <utility/structure.hpp>

#include <cstdint>

namespace chester
{
	namespace common
	{
		struct Header
		{
			uint16_t size;
			uint16_t id;

			template <typename S>
			friend void serialize(S & s, const Header & x)
			{
				using utility::serialize;

				serialize(s, x.size);
				serialize(s, x.id);
			}
			template <typename S>
			friend void structure(S & s, Header & x)
			{
				using utility::structure;

				structure(s, x.size);
				structure(s, x.id);
			}
		};
	}
}

#endif /* CHESTER_COMMON_HEADER_HPP */
