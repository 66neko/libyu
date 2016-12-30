#pragma once
#include <atomic>
#include <memory>
#include <functional>
#include <mutex>
#include "spinlock.hpp"

#define _THREADSAFE namespace threadsafe {
#define _END }

_THREADSAFE

template<class T,class V>
class list_iter
{
public:
	explicit list_iter(T val):
		value_{val}
	{
	}

	~list_iter()
	{
	}

	bool operator != (const list_iter& other) const
	{
		return (this->value_) != (other.value_);
	}
	V& operator* () const
	{
		return *(value_->data);
	}
	const list_iter& operator++ ()
	{
		value_ = value_->next;
		return *this;
	}
private:
	T value_;
};



template<class T>
class sortlist
{
	struct Tnode
	{
		std::shared_ptr<T> data;
		std::shared_ptr<Tnode> next;
		Tnode(T const & right) :
			data(std::make_shared<T>(right)),
			next{  }
		{
		}
		Tnode() :
			data{  },
			next{  }
		{
		}
	};
	using Myiter = list_iter<std::shared_ptr<Tnode>,T>;
private:
	sortlist(const sortlist<T>&) {}
public:
	explicit sortlist(std::function<bool(const T&, const T&)> compare = [](const T& l, const T&r)->bool {return l < r; }) :
		m_head{ std::make_shared<Tnode>() },
		m_compare{compare}
	{
	}

	~sortlist()
	{
		std::shared_ptr<Tnode> p{ m_head->next };
		while (p != nullptr)
		{
			std::shared_ptr<Tnode> sp{ p };
			p = p->next;
			sp->next = nullptr;
		}
	}

	void push(T const& data) {
		std::shared_ptr<Tnode> const new_node{ std::make_shared<Tnode>(data) };
		std::shared_ptr<Tnode> prev{ m_head };
		while (prev->next)
		{
			if (!m_compare(*(prev->next->data), data))
			{
				break;
			}
			prev = prev->next;
		}
		while (true)
		{
			
#ifdef _GCC_LINUX
			std::lock_guard<threadsafe::spin_lock> lock{ sp };
			std::shared_ptr<Tnode> next{ prev->next };
			new_node->next = next;
			prev->next = new_node;
			return;
#else
			std::shared_ptr<Tnode> next{ prev->next };
			new_node->next = next;
			if (std::atomic_compare_exchange_strong(&(prev->next), &next, new_node))
			{
				return;
			}
#endif // _GCC_LINUX	
		}
	}

	void pop() {
		std::shared_ptr<Tnode> old_head{ m_head };
#ifdef _GCC_LINUX
		std::lock_guard<threadsafe::spin_lock> lock{ sp };
		//while (old_head && !__sync_bool_compare_and_swap(m_head, *(old_head.get()), *((old_head->next).get())));
		if (old_head) 
		{
			m_head = old_head->next;
		}
		return;
#else
		while (old_head && !std::atomic_compare_exchange_weak(&m_head, &old_head, old_head->next));
#endif // _GCC_LINUX	
	}

	const T& front() const {
		return *(m_head->next->data);
	}

	Myiter begin() {
		return Myiter(m_head);
	}

	Myiter end() {
		std::shared_ptr<Tnode> p{ m_head->next };
		while (p->next != nullptr)
		{
			p = p->next;
		}
		return Myiter(p);
	}

	size_t size() {
		std::shared_ptr<Tnode> p{ m_head->next };
		size_t num{ 0 };
		while (p != nullptr)
		{
			num++;
			p = p->next;
		}
	
		
		return num;
	}

private:
	std::function<bool(const T&, const T&)> m_compare;
	std::shared_ptr<Tnode> m_head;
	threadsafe::spin_lock sp;
};


_END
