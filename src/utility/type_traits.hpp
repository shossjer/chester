
#ifndef CHESTER_UTILITY_TYPE_TRAITS_HPP
#define CHESTER_UTILITY_TYPE_TRAITS_HPP

#include <type_traits>

namespace chester
{
	namespace utility
	{
		template <std::size_t I>
		using index_constant = std::integral_constant<size_t, I>;

		template <typename T>
		struct type_is
		{
			using type = T;
		};

		template <typename ...Ts>
		struct type_list {};

		template <typename T, typename ...Ts>
		struct index_of;
		template <typename T, typename ...Ts>
		struct index_of<T, T, Ts...> : index_constant<0> {};
		template <typename T, typename T1, typename ...Ts>
		struct index_of<T, T1, Ts...> : index_constant<1 + index_of<T, Ts...>::value> {};

		template <typename T, typename List>
		struct index_in;
		template <typename T, typename ...Ts>
		struct index_in<T, type_list<Ts...>> : index_of<T, Ts...> {};
	}
}

#endif /* CHESTER_UTILITY_TYPE_TRAITS_HPP */
