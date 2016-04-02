
#ifndef CHESTER_UTILITY_DEBUG_HPP
#define CHESTER_UTILITY_DEBUG_HPP

#include <config.h>

#include <utility/spinlock.hpp>
#include <utility/stream.hpp>

#include <cstdlib>
#include <iostream>
#include <mutex>

#if MODE_DEBUG
/**
 * Asserts that the condition is true.
 */
# ifdef __GNUG__
#  define debug_assert(cond, ...) chester::utility::debug::instance().assert(__FILE__, __LINE__, #cond, cond, ##__VA_ARGS__)
# else
#  define debug_assert(cond, ...) chester::utility::debug::instance().assert(__FILE__, __LINE__, #cond, cond, __VA_ARGS__)
# endif
/**
 * Prints the arguments to the console (with a newline).
 *
 * \note Is thread-safe.
 *
 * \note Also prints the arguments to the debug log.
 */
# ifdef __GNUG__
#  define debug_printline(thing, ...) chester::utility::debug::instance().printline(__FILE__, __LINE__, thing, ##__VA_ARGS__)
# else
#  define debug_printline(thing, ...) chester::utility::debug::instance().printline(__FILE__, __LINE__, thing, __VA_ARGS__)
# endif
#else
/**
 * Does nothing.
 */
# define debug_assert(cond, ...)
/**
 * Does nothing.
 */
# define debug_print(thing, ...)
/**
 * Does nothing.
 */
# define debug_printline(thing, ...)
#endif

namespace chester
{
	namespace utility
	{
		/**
		 */
		class debug
		{
		private:
			using lock_t = chester::utility::spinlock;

		private:
			/**
			 */
			chester::utility::spinlock lock;
			/**
			 */
			std::ostream & stream;

		private:
			/**
			 */
			debug() :
				stream(std::cerr)
			{
			}

		public:
			/**
			 */
			template <std::size_t N, std::size_t M>
			void assert(const char (&file_name)[N], const int line_number, const char (&cond_string)[M], const bool cond_value)
			{
				if (cond_value) return;

				std::lock_guard<lock_t> guard{this->lock};
				chester::utility::to_stream(this->stream, file_name, "@", line_number, ":\n", cond_string, "\n");
				this->stream.flush();

				std::terminate();
			}
			/**
			 */
			template <std::size_t N, std::size_t M>
			void assert(const char (&file_name)[N], const int line_number, const char (&cond_string)[M], const bool cond_value, const std::string &comment)
			{
				if (cond_value) return;

				std::lock_guard<lock_t> guard{this->lock};
				utility::to_stream(this->stream, file_name, "@", line_number, ": ", cond_string, "\n", comment, "\n");
				this->stream.flush();

				std::terminate();
			}
			/**
			 */
			template <std::size_t N, typename ...Ps>
			void printline(const char (&file_name)[N], const int line_number, Ps && ...ps)
			{
				std::lock_guard<lock_t> guard{this->lock};
				utility::to_stream(this->stream, file_name, "@", line_number, ": ", std::forward<Ps>(ps)..., "\n");
				this->stream.flush();
			}

		public:
			/**
			 */
			static debug &instance()
			{
				static debug var;

				return var;
			}
		};
	}
}

#endif /* CHESTER_UTILITY_DEBUG_HPP */
