
#ifndef CHESTER_UTILITY_SERIALIZE_HPP
#define CHESTER_UTILITY_SERIALIZE_HPP

#include <cstddef>
#include <iterator>

namespace chester
{
	namespace utility
	{
		template <typename S>
		inline void serialize(S & s, const std::nullptr_t x)
		{
			s(x);
		}

		template <typename S>
		inline void serialize(S & s, const bool x)
		{
			s(x);
		}

		template <typename S>
		inline void serialize(S & s, const char x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const char16_t x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const char32_t x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const wchar_t x)
		{
			s(x);
		}

		template <typename S>
		inline void serialize(S & s, const signed char x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const unsigned char x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const short int x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const unsigned short int x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const int x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const unsigned int x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const long int x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const unsigned long int x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const long long int x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const unsigned long long int x)
		{
			s(x);
		}

		template <typename S>
		inline void serialize(S & s, const float x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const double x)
		{
			s(x);
		}
		template <typename S>
		inline void serialize(S & s, const long double x)
		{
			s(x);
		}

		template <typename S, typename T>
		inline auto serialize(S & s, const T & xs) ->
			decltype(std::begin(xs), std::end(xs), void())
		{
			for (auto && x : xs)
				serialize(s, x);
		}
	}
}

#endif /* CHESTER_UTILITY_SERIALIZE_HPP */
