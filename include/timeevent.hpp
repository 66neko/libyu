#pragma once
#include "tslist.hpp"
#include <thread>
#include <future>
#include <iostream>
class time_envent
{
	struct sEnvent
	{
		std::function<void()> callback;
		uint32_t timebase;
		explicit sEnvent(std::function<void()> cb, uint32_t tb) :
			callback{ cb },
			timebase{ tb }
		{
		}
	};
public:
	time_envent() :
		m_off{ false },
		m_t{ nullptr },
		m_time_sec{ 0 },
		m_time_base{ std::chrono::steady_clock::now() },
		m_ent{ [](const sEnvent& l,const sEnvent& r)->bool {return l.timebase <= r.timebase; } }
	{
		std::chrono::steady_clock::time_point t1{ std::chrono::steady_clock::now() };
		std::chrono::duration<uint32_t> time_span1{ std::chrono::duration_cast<std::chrono::duration<uint32_t>>(t1 - m_time_base) };
		m_time_sec = time_span1.count();
		m_t = std::make_shared<std::thread>([&]() {
			while (!m_off)
			{
				if (m_ent.size() != 0 && m_ent.front().timebase <= m_time_sec)
				{
					m_ent.front().callback();
					m_ent.pop();
					continue;
				}
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::chrono::steady_clock::time_point t2{ std::chrono::steady_clock::now() };
				std::chrono::duration<uint32_t> time_span{ std::chrono::duration_cast<std::chrono::duration<uint32_t>>(t2 - m_time_base) };
				m_time_sec.store(time_span.count());

			}
		});
	}

	~time_envent()
	{
		m_off = true;
		m_t->join();
	}

	template<class F,class... Args>
	auto commit(uint32_t time_sec, F && f, Args &&... args)->std::future<decltype(f(std::forward<Args>(args)...))> {
		using resType = decltype(f(std::forward<Args>(args)...));
		auto task = std::make_shared<std::packaged_task<resType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		std::chrono::steady_clock::time_point t1{ std::chrono::steady_clock::now() };
		std::chrono::duration<uint32_t> time_span1{ std::chrono::duration_cast<std::chrono::duration<uint32_t>>(t1 - m_time_base) };
		m_time_sec = time_span1.count();

		m_ent.push(sEnvent([task]() {(*task)(); }, time_sec+ m_time_sec.load()));
		std::future<resType> future = task->get_future();
		return future;
	}

private:
	bool m_off;
	std::shared_ptr<std::thread> m_t;
	std::atomic<uint32_t> m_time_sec;
	std::chrono::steady_clock::time_point m_time_base;
	threadsafe::sortlist<sEnvent> m_ent;
};
