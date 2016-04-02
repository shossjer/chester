
#include <config.h>

// #include <common/network/Server.hpp>
#include <common/network/initiate.hpp>

#if NETWORK_USE_SOCKET
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/select.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <unistd.h>
#elif NETWORK_USE_WINSOCK2
# include <winsock2.h>
#endif

#include <utility/debug.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <array>
//#include <condition_variable>
//#include <mutex>
//#include <thread>
#include <utility>
#include <vector>

namespace
{
#if NETWORK_USE_SOCKET
	using fd_t = int;
#endif

	struct timepoint_t
	{
#if NETWORK_USE_SOCKET
		struct timeval value;
#endif

		timepoint_t() = default;
		timepoint_t(std::size_t milliseconds)
		{
			value.tv_sec = milliseconds / 1000;
			value.tv_usec = (milliseconds % 1000) * 1000;
		}

		friend timepoint_t operator - (const timepoint_t & t1, const timepoint_t & t2)
		{
			const auto sec = t1.value.tv_sec - t2.value.tv_sec;
			const auto usec = t1.value.tv_usec - t2.value.tv_usec;

			timepoint_t res;
			res.value.tv_sec = sec - (usec < 0);
			res.value.tv_usec = usec + (usec < 0) * 1000000;
			return res;
		}
		friend bool operator < (const timepoint_t & t1, const timepoint_t & t2)
		{
			return std::tie(t1.value.tv_sec, t1.value.tv_usec) <
				   std::tie(t2.value.tv_sec, t2.value.tv_usec);
		}

		static timepoint_t now()
		{
			timepoint_t timepoint;
			// clock_gettime(CLOCK_MONOTONIC, &timepoint.value);
			gettimeofday(&timepoint.value, nullptr);
			return timepoint;
		}
		static timepoint_t zero()
		{
			timepoint_t timepoint;
			timepoint.value.tv_sec = 0;
			timepoint.value.tv_usec = 0;
			return timepoint;
		}
	};

	struct socket_t
	{
		fd_t fd;
		timepoint_t last_activity;
		timepoint_t accepted_time;

		socket_t() = default;
		socket_t(fd_t fd, timepoint_t timepoint) :
			fd(fd),
			last_activity(timepoint),
			accepted_time(timepoint)
		{}

		void close()
		{
#if NETWORK_USE_SOCKET
			::close(this->fd);
#elif NETWORK_USE_WINSOCK2
#endif
		}
		void shutdown()
		{
#if NETWORK_USE_SOCKET
			::shutdown(this->fd, SHUT_RDWR);
#elif NETWORK_USE_WINSOCK2
#endif
		}
	};

	using index_t = size_t;

	constexpr size_t max_connections = 5;

	struct timeout_t
	{
		const timepoint_t * timepoint_ptr;
		index_t index;

		timeout_t() = default;
		timeout_t(const socket_t & socket, index_t index) :
			timepoint_ptr(&socket.last_activity),
			index(index)
		{}

		friend timepoint_t operator - (const timeout_t & t1, const timepoint_t & t2)
		{
			return *t1.timepoint_ptr - t2;
		}
		friend bool operator < (const timepoint_t & t1, const timeout_t & t2)
		{
			return t1 < *t2.timepoint_ptr;
		}
		friend bool operator < (const timeout_t & t1, const timeout_t & t2)
		{
			return *t1.timepoint_ptr < *t2.timepoint_ptr;
		}
	};
	auto timeout_table = std::array<timeout_t, max_connections>{};
	auto timeout_amount = timepoint_t{3000};

	std::size_t nconnections = 0;
	auto sockets = std::array<socket_t, FD_SETSIZE>{};
	//auto free_indices = std::array<index_t, FD_SETSIZE>{};

	struct collection_t
	{
		fd_t max;
		fd_t min;
		fd_set set;

		void init(fd_t server_fd)
		{
			max = server_fd;
			min = server_fd;
			FD_ZERO(&this->set);
			FD_SET(server_fd, &this->set);
		}

		fd_t get_max() const
		{
			return this->max;
		}
		fd_t get_min() const
		{
			return this->min;
		}
		void get_set(fd_set & out) const
		{
			::memcpy(&out, &this->set, sizeof(fd_set));
		}

		void add(fd_t fd)
		{
			if (fd > max) max = fd;
			if (fd < min) min = fd;
			FD_SET(fd, &this->set);
		}
		/**
		 * prerequisite:
		 *   `fd` needs to have been added (at least once) before.
		 */
		void remove(fd_t fd)
		{
			if (fd == max)
			{
				while (max > min)
				{
					max--;
					if (FD_ISSET(max, &this->set)) break;
				}
			}
			if (fd == min)
			{
				while (min < max)
				{
					min++;
					if (FD_ISSET(min, &this->set)) break;
				}
			}
			FD_CLR(fd, &this->set);
		}
	};

	collection_t collection;
}

void run()
{
	// setup free indices
	//std::iota(std::begin(free_indices), std::end(free_indices), 0);

	// constructor
	fd_t server_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	// bind
	const uint16_t port = 27960;

	sockaddr_in address;
	{
		address.sin_family = AF_INET;
		address.sin_port = htons(port);
		address.sin_addr.s_addr = INADDR_ANY;
	}
	::bind(server_fd, (sockaddr *)&address, sizeof(address));
	// listen
	const std::size_t max_count = 3;

	::listen(server_fd, max_count);
	//
	collection.init(server_fd);

	while (true)
	{
		// select
		fd_t max_fd = collection.get_max();
		fd_t min_fd = collection.get_min();
		fd_set tmp_set;
		collection.get_set(tmp_set);
		timepoint_t timepoint;
		struct timeval * timeout_ptr = nullptr;
		if (nconnections > 0)
		{
			timeout_ptr = &timepoint.value;
			timepoint = timepoint_t::now() - timeout_amount;
			if (timepoint < timeout_table[0])
				timepoint = timeout_table[0] - timepoint;
			else
				timepoint = timepoint_t::zero();
		}

		const auto nfds = ::select(max_fd + 1, &tmp_set, nullptr, nullptr, timeout_ptr);

		// was there an error?
		if (nfds == -1)
		{
			if (errno == EINTR) continue;

			debug_printline("socket ", server_fd, ": select failed with ", errno, "(", strerror(errno), ")");
			break;
		}
		// did something happen?
		if (nfds > 0)
		{
			// did the server get a connection-request?
			if (FD_ISSET(server_fd, &tmp_set))
			{
				const fd_t client_fd = ::accept(server_fd, nullptr, nullptr);
				const auto ts = timepoint_t::now();

				if (client_fd == -1)
				{
					debug_assert(errno != EINTR);

					debug_printline("socket ", server_fd, ": accept failed with ", errno, "(", strerror(errno), ")");
				}
				else if (nconnections >= max_connections)
				{
					debug_printline("rejecting new socket ", client_fd, " due to max_connections reached");

					auto socket = socket_t{client_fd, ts};
					socket.shutdown();
					socket.close();
				}
				else
				{
					debug_assert(client_fd < FD_SETSIZE);

					debug_printline("accepting new socket ", client_fd);
					//const auto index = free_indices[nconnections];
					const auto index = size_t(client_fd);
					sockets[index] = socket_t{client_fd, ts};
					timeout_table[nconnections] = timeout_t{sockets[index], index};
					collection.add(client_fd);
					nconnections++;
				}
				FD_CLR(server_fd, &tmp_set);
			}
			for (fd_t fd = min_fd; fd < max_fd + 1; fd++)
			{
				if (!FD_ISSET(fd, &tmp_set)) continue;

				char buffer[256];
				const auto ret = ::recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
				const auto ts = timepoint_t::now();

				if (ret == -1)
				{
					debug_assert(errno != EINTR);

					debug_printline("socket ", fd, ": recv failed with ", errno, "(", strerror(errno), ")");
				}
				if (ret > 0)
				{
					sockets[fd].last_activity = ts;

					std::size_t written = 0;
					do
					{
						const auto kjhs = ::send(fd, buffer + written, ret - written, 0);
						if (kjhs == -1)
						{
							if (errno == EINTR) continue;

							debug_printline("socket ", fd, ": send failed with ", errno, "(", strerror(errno), ")");
							break;
						}
						written += kjhs;
					}
					while (written < std::size_t(ret));
				}
				if (ret == 0)
				{
					debug_printline("socket ", fd, " has started a shutdown");
					collection.remove(fd);
				}
			}
			std::sort(std::begin(timeout_table),
			          std::begin(timeout_table) + nconnections);
		}
		// did something timeout?
		//if (nfds == 0)
		{
			// debug_assert(nconnections > 0);

			std::size_t n = 0;
			while (n < nconnections)
			{
				const auto ts = timepoint_t::now() - timeout_amount;

				if (ts < timeout_table[n])
					break;

				const auto index = timeout_table[n].index;
				debug_printline("socket ", sockets[index].fd, " timeout, after ", (timeout_table[n] - (sockets[index].accepted_time - timeout_amount)).value.tv_sec, " seconds");
				sockets[index].shutdown();
				sockets[index].close();
				if (FD_ISSET(sockets[index].fd, &collection.set))
					collection.remove(sockets[index].fd);
				n++;
			}

			std::move(timeout_table.begin() + n,
			          timeout_table.begin() + nconnections,
			          timeout_table.begin());

			nconnections -= n;
		}
	}
}

// void handler()
// {
// 	std::lock_guard<std::mutex> idle_lock{idle_mutex};

// 	while (true)
// 	{
// 		idle_cv.wait(idle_lock);

// 		// TODO: break loop
// 		// if (time_to_exit) break;

// 		while (nclients > 0)
// 		{
// 			std::lock_guard<std::mutex> set_lock{set_mutex};

// 			struct timeval timeout = timeout_table[0].first.value;
// 			const auto nfds = select(int __nfds, fd_set *restrict __readfds, nullptr, nullptr, &timeout);
// 		}
// 	}
// }

// void func(std::size_t slot)
// {
// 	auto & client = clients[slot];
// 	try
// 	{
// 		uint8_t buffer[128];

// 		while (true)
// 		{
// 			const auto count = client.receive(buffer, sizeof(buffer));

// 			std::size_t written = 0;
// 			do
// 				written += client.send(buffer + written, count - written);
// 			while (written < count);
// 		}
// 	}
// 	catch (chester::common::network::recv_closed & x)
// 	{
// 		debug_printline("client ", slot, " recieved a shutdown");
// 	}
// 	debug_printline("client ", slot, " is shuting down");
// 	client.shutdown();
// }

// void run()
// {
// 	auto server = chester::common::network::Server{};
// 	server.bind(27960);
// 	server.listen(3);

// 	clients.resize(5);

// 	while (true)
// 	{
// 		auto client = server.accept();
// 		debug_assert(nclients < 5);
// 		const auto slot = free_slots[nclients++];
// 		clients[slot] = std::move(client);

// 		auto thread = std::thread{func, slot};
// 		thread.detach();
// 	}
// }

int main(const int argc, const char *const argv[])
{
	chester::common::network::initiate();
	run();
	// {
	// 	auto handler_thread = std::thread{handler};

	// 	handler_thread.join();
	// }
	chester::common::network::cleanup();

	return 0;
}
