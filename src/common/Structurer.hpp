
#ifndef CHESTER_COMMON_STRUCTURER_HPP
#define CHESTER_COMMON_STRUCTURER_HPP

#include <common/network/byte_order.hpp>

#include <utility/debug.hpp>

#include <algorithm>
#include <cstdint>
#include <vector>

namespace chester
{
	namespace common
	{
		class Structurer
		{
		private:
			std::vector<uint8_t> bytes;
			size_t at;

		public:
			Structurer(size_t capacity) : bytes(capacity), at(0) {}

		public:
			template <typename T>
			void operator () (T & x)
			{
				using namespace chester::common::network;

				type_t<T> z;
				debug_assert(this->at + sizeof(z) <= this->bytes.size());

				std::copy_n(&this->bytes[this->at], sizeof(z), reinterpret_cast<uint8_t *>(&z));
				this->at += sizeof(z);

				x = convert<T>::ntoh(z);
			}

		public:
			uint8_t * data()
			{
				return this->bytes.data();
			}
			bool empty() const
			{
				return this->at >= bytes.size();
			}
		};
	}
}

#endif /* CHESTER_COMMON_STRUCTURER_HPP */
