
#include "initiate.hpp"

#include <config.h>

#include <utility/debug.hpp>
#include <utility/string.hpp>

#if CRYPTO_USE_WOLFSSL
# include <cyassl/ssl.h>
#endif
#if NETWORK_USE_WINSOCK2
# include <winsock2.h>
#endif

namespace
{
	volatile bool is_initiated = false;
}

namespace chester
{
	namespace common
	{
		namespace network
		{
			bool isInitiated()
			{
				return is_initiated;
			}

			void cleanup()
			{
				debug_assert(is_initiated);

#if CRYPTO_USE_WOLFSSL
				CyaSSL_Cleanup();
#endif
#if NETWORK_USE_WINSOCK2
				if (WSACleanup())
				{
					throw std::runtime_error(utility::concat("WSACleanup failed with ", WSAGetLastError(), "."));
				}
#endif
				is_initiated = false;
			}

			void initiate()
			{
				debug_assert(!is_initiated);

#if CRYPTO_USE_WOLFSSL
				{
					const int err = CyaSSL_Init();

					if (err != SSL_SUCCESS)
					{
						throw std::runtime_error(utility::concat("CyaSSL_Init failed with ", err, "."));
					}
				}
#endif
#if NETWORK_USE_WINSOCK2
				{
					WSADATA wsadata;

					const int err = WSAStartup(0x0202, &wsadata);

					if (err)
					{
						CyaSSL_Cleanup();
						throw std::runtime_error(utility::concat("WSAStartup failed with ", err, "."));
					}

					if (wsadata.wVersion != 0x0202)
					{
						WSACleanup();
						CyaSSL_Cleanup();
						throw std::runtime_error("Wrong WSA version.");
					}
				}
#endif
				is_initiated = true;
			}
		}
	}
}
