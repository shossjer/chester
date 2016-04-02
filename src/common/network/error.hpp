
#ifndef CHESTER_COMMON_NETWORK_ERROR_HPP
#define CHESTER_COMMON_NETWORK_ERROR_HPP

#include <config.h>

#if NETWORK_USE_SOCKET
#include <cerrno>
#elif NETWORK_USE_WINSOCK2
#include <winsock2.h>
#endif

#include <stdexcept>
#include <string>

namespace chester
{
	namespace common
	{
		namespace network
		{
			/**
			 */
			class error : public std::runtime_error
			{
			public:
				explicit error(const std::string & arg) : runtime_error(arg) {}
				virtual ~error() = default;
			};

			/**
			 * \return The last error code.
			 */
			inline int error_code()
			{
#if NETWORK_USE_SOCKET
				return errno;
#elif NETWORK_USE_WINSOCK2
				return WSAGetLastError();
#endif
			}
		}
	}
}

#endif /* CHESTER_COMMON_NETWORK_ERROR_HPP */
