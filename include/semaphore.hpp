/*
*	semaphore - libyu
*	copyright - libyu
*	date - 2017-5-7
*	github - https://github.com/66neko/libyu
*/
#pragma once

#include <mutex>
#include <condition_variable>

#define _LIBYU namespace libyu {
#define _END }

_LIBYU

class semaphore {
public:
	semaphore(int value = 1) : count{ value }, wakeups{ 0 } {}

	void wait() {
		std::unique_lock<std::mutex> lock{ mutex };
		if (--count<0) {
			condition.wait(lock, [&]()->bool { return wakeups>0; });
			--wakeups;
		}
	}

	template<class _Rep, class _Period>
	bool wait_for(const std::chrono::duration<_Rep, _Period>& _time) {
		std::unique_lock<std::mutex> lock{ mutex };
		bool ret{ true };
		if (--count<0) {
			ret = condition.wait_for(lock, _time, [&]()->bool { return wakeups>0; });
			--wakeups;
		}
		return ret;
	}
	void signal() {
		std::lock_guard<std::mutex> lock{ mutex };
		if (++count <= 0) {
			++wakeups;
			condition.notify_one();
		}
	}

private:
	int count;
	int wakeups;
	std::mutex mutex;
	std::condition_variable condition;
};

_END