#include "Highscores.h"
#include "File.h"

void Highscores::Initialize()
{
	using namespace dae;

	file = File::OpenFile("highscores.sco");
	if (file) {
		Scores = file.ReadObject<HallOfFame>();
	}
	else {
		File::CreateFile("highscores.sco");
		file = File::OpenFile("highscores.sco");
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

bool Highscores::TryAddScore(const char* name, int score)
{
	for (int i = 0; i < 10; ++i) {
		if (Scores.Players[i].score < score) {

			HallOfFame next;

			memcpy(&next.Players, &Scores.Players, sizeof(HallOfFame::Player) * 10);
			memcpy(&next.Players[i + 1], &Scores.Players[i], sizeof(HallOfFame::Player) * (9 - i));
			memcpy(next.Players[i].name, name, 3);
			next.Players[i].score = score;
			Scores = next;

			Save();
			return true;
		}
	}
	return false;
}
