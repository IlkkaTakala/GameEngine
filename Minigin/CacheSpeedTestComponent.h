#pragma once
#include "BaseComponent.h"
#include <thread>

namespace dae {
	/*
	
	This component uses threads but is by no means thread safe. Use with caution.
	
	*/
class CacheSpeedTestComponent : public BaseComponent
{
	COMPONENT(CacheSpeedTestComponent)

public:
	
	void Recalculate();
	bool HasData() const { return hasData; }
	bool IsCalculating() const { return isCalculating; }
	std::vector<float>& GetData();
	void SetTestType(int newType) { type = newType; }
	void SetSteps(int steps) { maxSteps = steps; }
	int* GetStepsPtr() { return &maxSteps; }

	void OnDestroyed() override;

private:

	void Calculation();

	bool isCalculating{ false };
	bool hasData{ false };
	bool shouldThreadClose{ false };
	int type{ 0 };
	int maxSteps{ 10 };
	std::thread* worker{ nullptr };

	std::vector<float> data;
};

}