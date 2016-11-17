
#ifndef CHESTER_COMMON_READER_HPP
#define CHESTER_COMMON_READER_HPP

#include <config.h>

#include "Structurer.hpp"

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
		class Reader
		{
		private:
			HANDLE h;

		public:
			Reader(HANDLE h) : h(h) {}

		public:
			template <typename T>
			bool operator () (T & d, const size_t size)
			{
				chester::common::Structurer sd(size);

				DWORD count;
				/*const auto ret = */ReadFile(this->h, sd.data(), size, &count, NULL);

				structure(sd, d);
				debug_assert(sd.empty());

				return count > 0;
			}
			void operator () (uint8_t *const bytes, const std::size_t count)
			{
				std::size_t amount = 0;
				do
				{
					DWORD many;
					/*const auto ret = */ReadFile(this->h, bytes + amount, count - amount, &many, NULL);
					amount += many;
				}
				while (amount < count);
			}
		};
#else
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
#endif
	}
}

#endif /* CHESTER_COMMON_READER_HPP */
