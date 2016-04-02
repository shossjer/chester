
#ifndef CHESTER_COMMON_NETWORK_SERVER_HPP
#define CHESTER_COMMON_NETWORK_SERVER_HPP

#include <config.h>

#include "Client.hpp"
#include "error.hpp"
#include "initiate.hpp"
#include "Socket.hpp"

#include <utility/debug.hpp>
#include <utility/string.hpp>

#if NETWORK_USE_SOCKET
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#elif NETWORK_USE_WINSOCK2
#include <winsock2.h>
#endif

#include <cstdint>

namespace chester
{
	namespace common
	{
		namespace network
		{
			/**
			 */
			class Server : public Socket
			{
			private:
#if NETWORK_USE_SOCKET
				/**
				 */
				typedef struct sockaddr_in sockaddr_in;
				/**
				 */
				typedef struct sockaddr sockaddr;
#elif NETWORK_USE_WINSOCK2
				/**
				 */
				typedef SOCKADDR_IN sockaddr_in;
				/**
				 */
				typedef SOCKADDR sockaddr;
#endif

			public:
				/**
				 */
				Client accept();
				/**
				 */
				void bind(const uint16_t port);
				/**
				 */
				void listen(const std::size_t max_count);

			};
		}
	}

	namespace common
	{
		namespace network
		{
			inline Client Server::accept()
			{
				debug_assert(fd != Socket::invalid);
				debug_assert(isInitiated());

#if NETWORK_USE_SOCKET
				int ret;
			repeat:
				ret = ::accept(fd, NULL, NULL);
#elif NETWORK_USE_WINSOCK2
				const int ret = ::accept(fd, NULL, NULL);
#endif

				if (ret == Socket::invalid)
				{
#if NETWORK_USE_SOCKET
					// repeat if we got interrupted
					if (errno == EINTR) goto repeat;
#endif
					throw error(utility::concat("Failed to accept socket with ", error_code(), "."));
				}
				return Client{ret};
			}
			inline void Server::bind(const uint16_t port)
			{
				debug_assert(fd != Socket::invalid);
				debug_assert(isInitiated());

				sockaddr_in address;
				{
					address.sin_family = AF_INET;
					address.sin_port = htons(port);
					address.sin_addr.s_addr = INADDR_ANY;
				}
				const int ret = ::bind(fd, (sockaddr *)&address, sizeof(address));

				if (ret == Socket::err) throw error(utility::concat("Failed to bind socket with ", error_code(), "."));
			}
			inline void Server::listen(const std::size_t max_count)
			{
				debug_assert(fd != Socket::invalid);
				debug_assert(isInitiated());

				const int ret = ::listen(fd, max_count);

				if (ret == Socket::err) throw error(utility::concat("Failed to listen socket with ", error_code(), "."));
			}
		}
	}
}

#endif /* CHESTER_COMMON_NETWORK_SERVER_HPP */
