#pragma once

namespace dae
{

class Command
{
public:
	explicit Command();

	void operator()() {
		internal_execute();
	}
protected:
	virtual void OnExecute() = 0;
private:

	void internal_execute();
};

}