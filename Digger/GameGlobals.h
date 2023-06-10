#pragma once
#include "Singleton.h"
#include "Delegates.h"

enum class GameType
{
	Single,
	Coop,
	Pvp,
	None,
};

class GameGlobals : public dae::Singleton<GameGlobals>
{
public:
	char* GetPlayerName(int user) { return (char*)PlayerNames[user]; }
	GameType GetType() const { return Type; }
	void SetType(GameType type) { Type = type; }

	int GetScore() const { return Score; }
	void AddScore(int value) { Score += value; ScoreGained.Broadcast(Score); }
	void SetScore(int value) { Score = value; ScoreGained.Broadcast(Score); }

	dae::MulticastDelegate<int> ScoreGained;

private:
	GameType Type{};
	char PlayerNames[2][4];
	int Score;
};

