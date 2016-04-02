
#ifndef CHESTER_COMMON_NETWORK_CLIENT_HPP
#define CHESTER_COMMON_NETWORK_CLIENT_HPP

#include <config.h>

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
			class recv_closed : public error
			{
			public:
				virtual ~recv_closed() = default;
				explicit recv_closed(const std::string & arg) : error(arg) {}
			};

			/**
			 */
			class Client : public Socket
			{
				friend class Server;

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
				Client() = default;
			private:
				/**
				 */
				Client(const fd_t fd);

			public:
				/**
				 */
				fd_t socket() const { return this->fd; }
				/**
				 * \note Blocking operation.
				 *
				 * \return The number of bytes received. This value may be less than \c size.
				 */
				std::size_t receive(uint8_t *const data, const std::size_t size) const;
				/**
				 * \note Blocking operation.
				 *
				 * \return The number of bytes sent. This value may be less than \c size.
				 */
				std::size_t send(const uint8_t *const data, const std::size_t size) const;

			public:
				/**
				 */
				void connect(const char *const ip, const uint16_t port);
				/**
				 */
				void shutdown() const;
			private:
				/**
				 */
				void shutdown(const int how) const;

			};
		}
	}

	namespace common
	{
		namespace network
		{
			inline Client::Client(const fd_t fd) : Socket(fd)
			{
			}

			inline std::size_t Client::receive(uint8_t *const data, const std::size_t size) const
			{
				debug_assert(this->fd != Socket::invalid);
				debug_assert(isInitiated());

#if NETWORK_USE_SOCKET
				int count;
			repeat:
				count = ::recv(this->fd, data, size, 0);
#elif NETWORK_USE_WINSOCK2
				const int count = ::recv(this->fd, (char *)data, size, 0);
#endif

				if (count == Socket::err)
				{
#if NETWORK_USE_SOCKET
					// repeat if we got interrupted
					if (errno == EINTR) goto repeat;
#endif
					throw error(utility::concat("Failed to receive socket with ", error_code(), "."));
				}
				if (count == 0)
				{
					// shuting down the read-part might not be necessary for gracefull shutdown
#if NETWORK_USE_SOCKET
					this->shutdown(SHUT_RD);
#elif NETWORK_USE_WINSOCK2
					this->shutdown(SD_RECEIVE);
#endif
					// maybe the best option here is just to throw an exception
					throw recv_closed("");
				}
				return count;
			}
			inline std::size_t Client::send(const uint8_t *const data, const std::size_t size) const
			{
				debug_assert(this->fd != Socket::invalid);
				debug_assert(isInitiated());

#if NETWORK_USE_SOCKET
				int count;
			repeat:
				count = ::send(this->fd, data, size, 0);
#elif NETWORK_USE_WINSOCK2
				const int count = ::send(this->fd, (char *)data, size, 0);
#endif
				if (count == Socket::err)
				{
#if NETWORK_USE_SOCKET
					// repeat if we got interrupted
					if (errno == EINTR) goto repeat;
#endif
					throw error(utility::concat("Failed to send socket with ", error_code(), "."));
				}
				return count;
			}

			inline void Client::connect(const char *const ip, const uint16_t port)
			{
				debug_assert(this->fd != Socket::invalid);
				debug_assert(isInitiated());

				sockaddr_in address;
				{
					address.sin_family = AF_INET;
					address.sin_port = htons(port);
					address.sin_addr.s_addr = inet_addr(ip); // Avoid  its  use  in  favor  of  inet_aton(), inet_pton(3), or getaddrinfo(3) which provide a cleaner way to indicate error return. -- MAN
				}
				const int ret = ::connect(this->fd, (sockaddr *)&address, sizeof(address));

				if (ret == Socket::err) throw error(utility::concat("Failed to connect socket with ", error_code(), "."));
			}
			inline void Client::shutdown() const
			{
#if NETWORK_USE_SOCKET
				this->shutdown(SHUT_WR);
#elif NETWORK_USE_WINSOCK2
				this->shutdown(SD_SEND);
#endif
			}

			inline void Client::shutdown(const int how) const
			{
				debug_assert(this->fd != Socket::invalid);
				debug_assert(isInitiated());

				const int ret = ::shutdown(this->fd, how);

				if (ret == Socket::err) throw error(utility::concat("Failed to shutdown socket with ", error_code(), "."));
			}
		}
	}
}

#endif /* CHESTER_COMMON_NETWORK_CLIENT_HPP */
