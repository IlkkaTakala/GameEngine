#pragma once

namespace dae
{
class GameObject;

class CommandHandlerBase
{
public:
	virtual void Execute() = 0;
};

template<class T>
class CommandHandler : private CommandHandlerBase
{
public:
	explicit CommandHandler(T object) : data(object) {

	}

	virtual void Execute() override {
		data();
	}

private:

	T data;
};

class Command
{
	GameObject* Owner;
public:
	explicit Command(GameObject* owner);

	void operator()() {
		internal_execute();
	}
protected:
	virtual void OnExecute() = 0;
private:

	void internal_execute();
};

}