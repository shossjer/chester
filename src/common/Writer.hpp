
#ifndef CHESTER_COMMON_WRITER_HPP
#define CHESTER_COMMON_WRITER_HPP

#include <config.h>

#include "Header.hpp"
#include "Serializer.hpp"
#include "messages.hpp"

#include <utility/debug.hpp>

#if CLIENT_IS_WIN32
# include <windows.h>
#else
# include <unistd.h>
#endif

namespace chester
{
	namespace common
	{
#if CLIENT_IS_WIN32
		class Writer
		{
		private:
			HANDLE h;

		public:
			Writer(HANDLE h) : h(h) {}

		public:
			template <typename T>
			void operator () (const T & m)
			{
				chester::common::Serializer sm;
				serialize(sm, m);

				chester::common::Serializer sh;
				serialize(sh, chester::common::Header{uint16_t(sm.size()), msg::id_of<T>::value});
				debug_assert(sh.size() == sizeof(chester::common::Header));

				DWORD count1;
				/*const auto ret1 = */WriteFile(this->h, sh.data(), sizeof(chester::common::Header), &count1, NULL);
				debug_assert(count1 == sizeof(chester::common::Header));
				DWORD count2;
				/*const auto ret2 = */WriteFile(this->h, sm.data(), sm.size(), &count2, NULL);
				debug_assert(count2 == sm.size());
			}
			void operator () (const uint8_t *const bytes, const std::size_t count)
			{
				std::size_t written = 0;
				do
				{
					DWORD many;
					/*const auto ret = */WriteFile(this->h, bytes + written, count - written, &many, NULL);
					written += many;
				}
				while (written < count);
			}
		};
#else
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
				const auto count2 = ::write(this->fd, sm.data(), sm.size());
				debug_assert(count2 == sm.size());
			}
			void operator () (const uint8_t *const bytes, const std::size_t count)
			{
				std::size_t written = 0;
				do
				{
					written += ::write(this->fd, bytes + written, count - written);
				}
				while (written < count);
			}
		};
#endif
	}
}

#endif /* CHESTER_COMMON_WRITER_HPP */
