
#ifndef CHESTER_UTILITY_SPINLOCK_HPP
#define CHESTER_UTILITY_SPINLOCK_HPP

#include <atomic>

namespace chester
{
	namespace utility
	{
		class spinlock
		{
		private:
			std::atomic_flag flag;

		public:
			spinlock() :
				flag{ATOMIC_FLAG_INIT}
			{
			}

		public:
			void lock()
			{
				while (this->flag.test_and_set());
			}
			void unlock()
			{
				this->flag.clear();
			}
		};
	}
}

#endif /* CHESTER_UTILITY_SPINLOCK_HPP */
