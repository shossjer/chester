
#ifndef CHESTER_COMMON_WRITER_HPP
#define CHESTER_COMMON_WRITER_HPP

#include "Header.hpp"
#include "Serializer.hpp"
#include "messages.hpp"

#include <utility/debug.hpp>

#include <unistd.h>

namespace chester
{
	namespace common
	{
		class Writer
		{
		private:
			const int fd;

		public:
			Writer(int fd) : fd(fd) {}

		public:
			template <typename T>
			void operator () (const T & m)
			{
				chester::common::Serializer sm;
				serialize(sm, m);

				chester::common::Serializer sh;
				serialize(sh, chester::common::Header{uint16_t(sm.size()), msg::id_of<T>::value});
				debug_assert(sh.size() == sizeof(chester::common::Header));

				const auto count1 = ::write(this->fd, sh.data(), sizeof(chester::common::Header));
				debug_assert(count1 == sizeof(chester::common::Header));
				//debug_printline("wrote ", count1, "/", sizeof(chester::common::Header), " bytes");
				const auto count2 = ::write(this->fd, sm.data(), sm.size());
				debug_assert(count2 == sm.size());
				//debug_printline("wrote ", count2, "/", sm.size(), " bytes");
			}
			void operator () (const uint8_t *const bytes, const std::size_t count)
			{
				std::size_t written = 0;
				do
				{
					written += ::write(this->fd, bytes + written, count - written);
					//debug_printline("wrote ", written, "/", count, " bytes");
				}
				while (written < count);
			}
		};
	}
}

#endif /* CHESTER_COMMON_WRITER_HPP */
