#pragma once
#include <atomic>

#define _THREADSAFE namespace threadsafe {
#define _END }

_THREADSAFE

template<class T>
class atomic_template
{
public:
	template<class... Args>
	atomic_template(Args&&... args) :
		m_value{ args... },
		m_flag{ ATOMIC_FLAG_INIT }
	{
	}

	~atomic_template()
	{
	}

	T& load() {
		return m_value;
	}

	template<class F, class... Args>
	auto Do(F&& f, Args&&... args)->decltype(f(m_value, std::forward<Args>(args)...)) {
		while (m_flag.test_and_set());
		using ResType = decltype(f(m_value, std::forward<Args>(args)...));
		ResType res{ f(m_value, std::forward<Args>(args)...) };
		m_flag.clear();
		return res;
	}

	template<class _VOID_IF, class F, class... Args>
	auto Do(F&& f, Args&&... args)->decltype(f(m_value, std::forward<Args>(args)...)) {
		while (m_flag.test_and_set());
		using ResType = decltype(f(m_value, std::forward<Args>(args)...));
		f(m_value, std::forward<Args>(args)...);
		m_flag.clear();
		return;
	}

private:
	std::atomic_flag m_flag;
	T m_value;
};

_END