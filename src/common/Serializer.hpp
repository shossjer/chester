
#ifndef CHESTER_COMMON_SERIALIZER_HPP
#define CHESTER_COMMON_SERIALIZER_HPP

#include <common/network/byte_order.hpp>

#include <utility/debug.hpp>

#include <cstdint>
#include <vector>

namespace chester
{
	namespace common
	{
		class Serializer
		{
		private:
			std::vector<uint8_t> bytes;

		public:
			template <typename T>
			void operator () (const T & x)
			{
				using namespace chester::common::network;

				const type_t<T> z = convert<T>::hton(x);

				this->bytes.insert(this->bytes.end(),
				                   reinterpret_cast<const uint8_t *>(&z),
				                   reinterpret_cast<const uint8_t *>(&z) + sizeof(z));
				debug_assert(this->bytes.size() < 0x10000); // arbitrary
			}

		public:
			const uint8_t * data() const
			{
				return this->bytes.data();
			}
			size_t size() const
			{
				return this->bytes.size();
			}

			void clear()
			{
				this->bytes.clear();
			}
		};
	}
}

#endif /* CHESTER_COMMON_SERIALIZER_HPP */
