#pragma once
#include "Singleton.h"
#include "File.h"

struct HallOfFame
{
	struct Player {
		char name[4]{};
		int score{};
	} Players[10];
};

class Highscores : public dae::Singleton<Highscores>
{
public:

	void Initialize();

	void Save();

	const HallOfFame& GetScores();

	bool TryAddScore(const char* name, int score);

private:

	dae::File file;
	HallOfFame Scores;
};

