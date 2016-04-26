
#ifndef CHESTER_COMMON_NETWORK_BYTE_ORDER_HPP
#define CHESTER_COMMON_NETWORK_BYTE_ORDER_HPP

#include <config.h>

#include <utility/type_traits.hpp>

#include <cstdint>

namespace chester
{
	namespace common
	{
		namespace network
		{
			template <class T>
			struct type_impl;
			template <>
			struct type_impl<uint8_t> : utility::type_is<uint8_t> {};
			template <>
			struct type_impl<uint16_t> : utility::type_is<uint16_t> {};
			template <>
			struct type_impl<uint32_t> : utility::type_is<uint32_t> {};
			template <>
			struct type_impl<uint64_t> : utility::type_is<uint64_t> {};
			template <>
			struct type_impl<int8_t> : utility::type_is<uint8_t> {};
			template <>
			struct type_impl<int16_t> : utility::type_is<uint16_t> {};
			template <>
			struct type_impl<int32_t> : utility::type_is<uint32_t> {};
			template <>
			struct type_impl<int64_t> : utility::type_is<uint64_t> {};
			template <>
			struct type_impl<float> : utility::type_is<uint32_t> {};
			template <>
			struct type_impl<double> : utility::type_is<uint64_t> {};

			template <class T>
			using type_t = typename type_impl<T>::type;

			template <class T>
			struct convert;
			template <>
			struct convert<uint8_t>
			{
				static type_t<uint8_t> hton(const uint8_t x) { return x; }
				static uint8_t ntoh(const type_t<uint8_t> x) { return x; }
			};
#if WORDS_BIGENDIAN
			template <>
			struct convert<uint16_t>
			{
				static type_t<uint16_t> hton(const uint16_t x) { return x; }
				static uint16_t ntoh(const type_t<uint16_t> x) { return x; }
			};
			template <>
			struct convert<uint32_t>
			{
				static type_t<uint32_t> hton(const uint32_t x) { return x; }
				static uint32_t ntoh(const type_t<uint32_t> x) { return x; }
			};
			template <>
			struct convert<uint64_t>
			{
				static type_t<uint64_t> hton(const uint64_t x) { return x; }
				static uint64_t ntoh(const type_t<uint64_t> x) { return x; }
			};
#else
			template <>
			struct convert<uint16_t>
			{
				static type_t<uint16_t> hton(const uint16_t x) {
					return (x >> 8) | (x << 8);
				}
				static uint16_t ntoh(const type_t<uint16_t> x) {
					return (x >> 8) | (x << 8);
				}
			};
			template <>
			struct convert<uint32_t>
			{
				static type_t<uint32_t> hton(uint32_t x) {
					x = (x >> 16) | (x << 16);
					x = ((x >> 8) & 0x00ff00ff) | ((x << 8) & 0xff00ff00);
					return x;
				}
				static uint32_t ntoh(type_t<uint32_t> x) {
					x = (x >> 16) | (x << 16);
					x = ((x >> 8) & 0x00ff00ff) | ((x << 8) & 0xff00ff00);
					return x;
				}
			};
			template <>
			struct convert<uint64_t>
			{
				static type_t<uint64_t> hton(uint64_t x) {
					x = (x >> 32) | (x << 32);
					x = ((x >> 16) & 0x0000ffff0000ffff) | ((x << 16) & 0xffff0000ffff0000);
					x = ((x >> 8) & 0x00ff00ff00ff00ff) | ((x << 8) & 0xff00ff00ff00ff00);
					return x;
				}
				static uint64_t ntoh(type_t<uint64_t> x) {
					x = (x >> 32) | (x << 32);
					x = ((x >> 16) & 0x0000ffff0000ffff) | ((x << 16) & 0xffff0000ffff0000);
					x = ((x >> 8) & 0x00ff00ff00ff00ff) | ((x << 8) & 0xff00ff00ff00ff00);
					return x;
				}
			};
#endif
			template <>
			struct convert<int8_t>
			{
				static type_t<int8_t> hton(const int8_t x) { return convert<uint8_t>::hton(x); }
				static int8_t ntoh(const type_t<int8_t> x) { return convert<uint8_t>::ntoh(x); }
			};
			template <>
			struct convert<int16_t>
			{
				static type_t<int16_t> hton(const int16_t x) { return convert<uint16_t>::hton(x); }
				static int16_t ntoh(const type_t<int16_t> x) { return convert<uint16_t>::ntoh(x); }
			};
			template <>
			struct convert<int32_t>
			{
				static type_t<int32_t> hton(const int32_t x) { return convert<uint32_t>::hton(x); }
				static int32_t ntoh(const type_t<int32_t> x) { return convert<uint32_t>::ntoh(x); }
			};
			template <>
			struct convert<int64_t>
			{
				static type_t<int64_t> hton(const int64_t x) { return convert<uint64_t>::hton(x); }
				static int64_t ntoh(const type_t<int64_t> x) { return convert<uint64_t>::ntoh(x); }
			};
			template <>
			struct convert<float>
			{
				static type_t<float> hton(const float x) {
					// return utility::memory::view<type_t<float>>(x);
					return *reinterpret_cast<const type_t<float> *>(&x);
				}
				static float ntoh(const type_t<float> x) {
					// return utility::memory::view<float>(x);
					return *reinterpret_cast<const float *>(&x);
				}
			};
			template <>
			struct convert<double>
			{
				static type_t<double> hton(const double x) {
					// return utility::memory::view<type_t<double>>(x);
					return *reinterpret_cast<const type_t<double> *>(&x);
				}
				static double ntoh(const type_t<double> x) {
					// return utility::memory::view<double>(x);
					return *reinterpret_cast<const double *>(&x);
				}
			};
		}
	}
}

#endif /* CHESTER_COMMON_NETWORK_BYTE_ORDER_HPP */
