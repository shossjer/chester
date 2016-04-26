
#ifndef CHESTER_COMMON_READER_HPP
#define CHESTER_COMMON_READER_HPP

#include "Structurer.hpp"

#include <utility/debug.hpp>

#include <unistd.h>

namespace chester
{
	namespace common
	{
		class Reader
		{
		private:
			const int fd;

		public:
			Reader(int fd) : fd(fd) {}

		public:
			template <typename T>
			bool operator () (T & d, const size_t size)
			{
				chester::common::Structurer sd(size);

				const auto count = ::read(this->fd, sd.data(), size);
				//debug_printline("read ", count, "/", size, " bytes");

				structure(sd, d);
				debug_assert(sd.empty());

				return count > 0;
			}
			void operator () (uint8_t *const bytes, const std::size_t count)
			{
				std::size_t amount = 0;
				do
				{
					amount += ::read(this->fd, bytes + amount, count - amount);
				}
				while (amount < count);
			}
		};
	}
}

#endif /* CHESTER_COMMON_READER_HPP */
