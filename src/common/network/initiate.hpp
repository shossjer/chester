
#ifndef CHESTER_COMMON_NETWORK_INITIATE_HPP
#define CHESTER_COMMON_NETWORK_INITIATE_HPP

#include <stdexcept>

namespace chester
{
	namespace common
	{
		namespace network
		{
			/**
			 */
			bool isInitiated();

			/**
			 * \exception std::logic_error When not initiated.
			 * \exception std::runtime_error When cleanup fails.
			 */
			void cleanup();
			/**
			 * \exception std::logic_error When already initiated.
			 * \exception std::runtime_error When initiation fails.
			 */
			void initiate();
		}
	}
}

#endif /* CHESTER_COMMON_NETWORK_INITIATE_HPP */
