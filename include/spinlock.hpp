/*

	spin lock
	by yume.liu
*/

#pragma once
#include <atomic>
#include <mutex>


#define _THREADSAFE namespace threadsafe {
#define _END }

_THREADSAFE

class spin_lock
{
public:
	spin_lock() :
		m_flag{ ATOMIC_FLAG_INIT }
	{
	}

	~spin_lock()
	{
	}

	void lock() {
		while (m_flag.test_and_set());
	}
	void unlock() {
		m_flag.clear();
	}

	bool try_lock() {
		return !m_flag.test_and_set();
	}
private:
	std::atomic_flag m_flag;
};


_END