#pragma once
#include <functional>
#include <list>

namespace dae {
	class GameObject;
template<typename ...Args>
class MulticastDelegate
{
public:
	typedef std::function<void(Args...)> DelegateType;
	struct DelegateHandle
	{
		friend class MulticastDelegate;
		DelegateHandle() : handle(), valid(false) { }
	private:
		DelegateHandle(typename const std::list<DelegateType>::const_iterator& in_handle) : handle(in_handle), valid(true) { }
		bool valid{ false };
		typename std::list<DelegateType>::const_iterator handle;
	};

	MulticastDelegate() {}
	MulticastDelegate(const MulticastDelegate& other) = delete;
	MulticastDelegate(MulticastDelegate&& other) = default;
	MulticastDelegate& operator=(const MulticastDelegate& other) = delete;
	MulticastDelegate& operator=(MulticastDelegate&& other) = default;

	void Broadcast(Args... args) {
		for (auto& c : Callbacks)
			c(args...);
	}

	// TODO: Binding should be bound to the lifetime of the given object, now it's up to the user
	DelegateHandle Bind(GameObject* /*object*/, const DelegateType& function) {
		Callbacks.emplace_back(function);
		return { --Callbacks.end() };
	}

	void Unbind(DelegateHandle& handle) {
		if (!handle.valid) return;
		handle.valid = false;
		Callbacks.erase(handle.handle);
	}

	void UnbindAll() {
		Callbacks.clear();
	}

private:

	std::list<DelegateType> Callbacks;
};

}