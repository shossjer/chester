
#ifndef CHESTER_COMMON_NETWORK_SOCKET_HPP
#define CHESTER_COMMON_NETWORK_SOCKET_HPP

#include <config.h>

#include "error.hpp"
#include "initiate.hpp"

#include <utility/debug.hpp>
#include <utility/string.hpp>

#if NETWORK_USE_SOCKET
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
#elif NETWORK_USE_WINSOCK2
# include <winsock2.h>
#endif

namespace chester
{
	namespace common
	{
		namespace network
		{
			/**
			 */
			class Socket
			{
			public:
#if NETWORK_USE_SOCKET
				/**
				 */
				typedef int fd_t;
#elif NETWORK_USE_WINSOCK2
				/**
				 */
				typedef SOCKET fd_t;
#endif

			protected:
#if NETWORK_USE_SOCKET
				/**
				 */
				static const fd_t err = -1;
				/**
				 */
				static const fd_t invalid = -1;
#elif NETWORK_USE_WINSOCK2
				/**
				 */
				static const fd_t err = SOCKET_ERROR;
				/**
				 */
				static const fd_t invalid = INVALID_SOCKET;
#endif

			protected:
				/**
				 */
				fd_t fd;

			public:
				/**
				 */
				Socket();
				/**
				 */
				Socket(const Socket &socket) = delete;
				/**
				 */
				Socket(Socket &&socket);
				/**
				 */
				virtual ~Socket();
			protected:
				/**
				 */
				Socket(const fd_t fd);

			public:
				/**
				 */
				Socket &operator = (const Socket &socket) = delete;
				/**
				 */
				Socket &operator = (Socket &&socket);

			public:
				/**
				 */
				bool isValid() const;

			private:
				/**
				 */
				void close();

			};
		}
	}

	namespace common
	{
		namespace network
		{
			inline Socket::Socket() :
#if MODE_DEBUG
				fd(isInitiated() ? ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) : throw std::logic_error("Is not initiated."))
#else
				fd(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
#endif
			{
				if (this->fd == Socket::invalid) throw error(utility::concat("Failed to create socket with ", error_code(), "."));
			}
			inline Socket::Socket(Socket &&socket) :
				fd(socket.fd)
			{
				socket.fd = Socket::invalid;
			}
			inline Socket::~Socket()
			{
				if (this->fd != Socket::invalid)
				{
					try
					{
						this->close();
					}
					catch (error &)
					{
						// one must never throw in a destructor
					}
				}
			}

			inline Socket::Socket(const fd_t fd) :
				fd(fd)
			{
			}

			inline Socket &Socket::operator = (Socket &&socket)
			{
				if (this->fd != Socket::invalid) this->close();

				this->fd = socket.fd;
				socket.fd = Socket::invalid;

				return *this;
			}

			inline bool Socket::isValid() const
			{
				return this->fd != Socket::invalid;
			}

			inline void Socket::close()
			{
				int ret;
			repeat:
#if NETWORK_USE_SOCKET
				ret = ::close(this->fd);
#elif NETWORK_USE_WINSOCK2
				ret = ::closesocket(this->fd);
#endif
				if (ret == Socket::err)
				{
					// repeat if we got interrupted
					if (errno == EINTR) goto repeat;

					throw error(utility::concat("Failed to close socket with ", error_code(), "."));
				}
			}
		}
	}
}

#endif /* CHESTER_COMMON_NETWORK_SOCKET_HPP */
