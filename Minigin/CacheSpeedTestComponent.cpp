#include "CacheSpeedTestComponent.h"

void dae::CacheSpeedTestComponent::Recalculate()
{
	if (!isCalculating) {
		isCalculating = true;
		hasData = false;
		if (worker) {
			worker->join();
			delete worker;
		}
		worker = new std::thread(&CacheSpeedTestComponent::Calculation, this);
	}
}

std::vector<float>& dae::CacheSpeedTestComponent::GetData()
{
	return data;
}

void dae::CacheSpeedTestComponent::OnDestroyed()
{
	shouldThreadClose = true;
	if (worker) worker->join();
	delete worker;
}

struct TestTransform
{
	float matrix[16] = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1,
	};
};

class TestGameObject3D
{
public:
	TestTransform transform{};
	int ID{ 0 };
};

class TestGameObject3DAlt
{
public:
	TestTransform* transform{ nullptr };
	int ID{ 0 };
};

void dae::CacheSpeedTestComponent::Calculation()
{
	std::map<int, std::vector<long long>> times;

	long arrLength = 20'000'000;

	switch (type)
	{
	case 0: {
		int* arr = new int[arrLength]();
		for (int c = 0; c < maxSteps && !shouldThreadClose; ++c) {

			int stepSize = 1;
			while (stepSize < 1025 && !shouldThreadClose) {

				memset(arr, 0, arrLength);
				const auto start = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < arrLength && !shouldThreadClose; i += stepSize)
					arr[i] *= 2;

				const auto end = std::chrono::high_resolution_clock::now();
				const auto total = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

				times[stepSize].push_back(total);

				stepSize += stepSize;
			}
		}
		delete[] arr;
	} break;

	case 1: {
		TestGameObject3D* arr = new TestGameObject3D[arrLength]();
		for (int c = 0; c < maxSteps && !shouldThreadClose; ++c) {

			int stepSize = 1;
			while (stepSize < 1025 && !shouldThreadClose) {

				memset(arr, 0, arrLength);
				const auto start = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < arrLength && !shouldThreadClose; i += stepSize)
					arr[i].ID *= 2;

				const auto end = std::chrono::high_resolution_clock::now();
				const auto total = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

				times[stepSize].push_back(total);

				stepSize += stepSize;
			}
		}
		delete[] arr;
	} break;

	case 2: {
		TestGameObject3DAlt* arr = new TestGameObject3DAlt[arrLength]();
		for (int c = 0; c < maxSteps && !shouldThreadClose; ++c) {

			int stepSize = 1;
			while (stepSize < 1025 && !shouldThreadClose) {

				memset(arr, 0, arrLength);
				const auto start = std::chrono::high_resolution_clock::now();
				for (int i = 0; i < arrLength && !shouldThreadClose; i += stepSize)
					arr[i].ID *= 2;

				const auto end = std::chrono::high_resolution_clock::now();
				const auto total = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

				times[stepSize].push_back(total);

				stepSize += stepSize;
			}
		}
		delete[] arr;
	} break;

	default:
		break;
	}
	
	data.clear();
	for (const auto& [step, timedata] : times) {
		int total = 0;
		for (const auto& time : timedata) {
			total += (int)time;
		}
		total = total / (int)timedata.size();

		data.push_back((float)total);
	}

	hasData = true;
	isCalculating = false;
	shouldThreadClose = false;
}
