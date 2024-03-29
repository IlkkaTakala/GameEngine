#pragma once
#include <string>
#include <functional>

namespace dae
{
	class StateManager;
	class Minigin
	{
	public:
		explicit Minigin(const std::string& dataPath);
		~Minigin();
		void Run(const std::function<void(StateManager*)>& load);

		Minigin(const Minigin& other) = delete;
		Minigin(Minigin&& other) = delete;
		Minigin& operator=(const Minigin& other) = delete;
		Minigin& operator=(Minigin&& other) = delete;

	private:

		StateManager* GameState;
	};
}