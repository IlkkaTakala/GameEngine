#pragma once
#include <functional>
#include <list>

namespace dae {

template<typename ...Args>
class MulticastDelegate
{
public:
	typedef std::function<void(Args...)> DelegateType;
	typedef const std::list<DelegateType>::const_iterator DelegateHandle;

	void Broadcast(Args... vals) {
		for (auto& c : Callbacks)
			c(vals...);
	}

	DelegateHandle Bind(const DelegateType& function) {
		Callbacks.emplace_back(function);
		return --Callbacks.end();
	}

	DelegateHandle Bind(void* object, void(*function)(Args...)) {
		DelegateType func = std::bind(function, object, Args...);
		Callbacks.push_back(func);
		return --Callbacks.end();
	}

	void Unbind(DelegateHandle& handle) {
		Callbacks.remove(handle);
	}

	void UnbindAll() {
		Callbacks.clear();
	}

private:

	std::list<DelegateType> Callbacks;
};

}