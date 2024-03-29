#include "Highscores.h"
#include "File.h"

const char* highScorePath = "../Data/highscores.sco";

void Highscores::Initialize()
{
	using namespace dae;

	file = File::OpenFile(highScorePath);
	if (file) {
		Scores = file.ReadObject<HallOfFame>();
	}
	else {
		File::CreateFile(highScorePath);
		file = File::OpenFile(highScorePath);
		Scores = HallOfFame();
	}
}

void Highscores::Save()
{
	if (file)
		file.WriteObject(Scores, 0);
}

const HallOfFame& Highscores::GetScores()
{
	return Scores;
}

bool Highscores::TryAddScore(const char* name, int score, GameType category)
{
	for (int i = 0; i < 10; ++i) {
		HallOfFame::Player* data = Scores.Players[(int)category];

		if (data[i].score < score) {

			HallOfFame next = Scores;
			HallOfFame::Player* target = next.Players[(int)category];

			//memcpy(target, data, sizeof(HallOfFame::Player) * 10);
			memcpy(&target[i + 1], &data[i], sizeof(HallOfFame::Player) * (9 - i));
			memcpy(target[i].name, name, 3);
			target[i].score = score;
			Scores = next;

			Save();
			return true;
		}
	}
	return false;
}
