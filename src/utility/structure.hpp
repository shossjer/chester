
#ifndef CHESTER_UTILITY_STRUCTURE_HPP
#define CHESTER_UTILITY_STRUCTURE_HPP

#include <cstddef>
#include <iterator>

namespace chester
{
	namespace utility
	{
		template <typename S>
		void structure(S & s, std::nullptr_t & x)
		{
			s(x);
		}

		template <typename S>
		void structure(S & s, bool & x)
		{
			s(x);
		}

		template <typename S>
		void structure(S & s, char & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, char16_t & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, char32_t & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, wchar_t & x)
		{
			s(x);
		}

		template <typename S>
		void structure(S & s, signed char & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, unsigned char & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, short int & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, unsigned short int & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, int & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, unsigned int & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, long int & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, unsigned long int & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, long long int & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, unsigned long long int & x)
		{
			s(x);
		}

		template <typename S>
		void structure(S & s, float & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, double & x)
		{
			s(x);
		}
		template <typename S>
		void structure(S & s, long double & x)
		{
			s(x);
		}

		template <typename S, typename T>
		inline auto structure(S & s, T & xs) ->
			decltype(std::begin(xs), std::end(xs), void())
		{
			for (auto && x : xs)
				serialize(s, x);
		}
	}
}

#endif /* CHESTER_UTILITY_STRUCTURE_HPP */
