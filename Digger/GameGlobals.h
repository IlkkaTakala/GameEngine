#pragma once
#include "Singleton.h"

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


private:
	GameType Type{};
	char PlayerNames[2][4];
};

