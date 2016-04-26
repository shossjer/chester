
#ifndef CHESTER_COMMON_MESSAGES_HPP
#define CHESTER_COMMON_MESSAGES_HPP

#include "Code.hpp"

#include <utility/serialize.hpp>
#include <utility/structure.hpp>
#include <utility/type_traits.hpp>

#include <vector>

namespace chester
{
	namespace common
	{
		namespace msg
		{
			struct ping_t
			{
				using this_type = ping_t;

				template <typename S>
				friend void serialize(S & s, const this_type & x)
				{
					using utility::serialize;
				}
				template <typename S>
				friend void structure(S & s, this_type & x)
				{
					using utility::structure;
				}
			};

			struct pull_t
			{
				using this_type = pull_t;

				chester::common::Code code;

				template <typename S>
				friend void serialize(S & s, const this_type & x)
				{
					using utility::serialize;
					serialize(s, x.code);
				}
				template <typename S>
				friend void structure(S & s, this_type & x)
				{
					using utility::structure;
					structure(s, x.code);
				}
			};
			struct push_t
			{
				using this_type = push_t;

				chester::common::Code code;
				uint32_t size;

				template <typename S>
				friend void serialize(S & s, const this_type & x)
				{
					using utility::serialize;
					serialize(s, x.code);
					serialize(s, x.size);
				}
				template <typename S>
				friend void structure(S & s, this_type & x)
				{
					using utility::structure;
					structure(s, x.code);
					structure(s, x.size);
				}
			};
			struct query_t
			{
				using this_type = query_t;

				std::vector<chester::common::Code> codes;

				template <typename S>
				friend void serialize(S & s, const this_type & x)
				{
					using utility::serialize;
					debug_assert(x.codes.size() < 0x100000000L);
					serialize(s, uint32_t(x.codes.size()));
					for (auto && code : x.codes)
						serialize(s, code);
				}
				template <typename S>
				friend void structure(S & s, this_type & x)
				{
					using utility::structure;
					uint32_t ncodes;
					structure(s, ncodes);
					x.codes.resize(ncodes);
					for (auto & code : x.codes)
						structure(s, code);
				}
			};

			using types = utility::type_list<ping_t,
			                                 pull_t,
			                                 push_t,
			                                 query_t>;

			template <typename T>
			using id_of = utility::index_in<T, types>;
		}
	}
}

#endif /* CHESTER_COMMON_MESSAGES_HPP */
