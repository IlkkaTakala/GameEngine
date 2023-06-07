#pragma once
#include "Singleton.h"

enum class GameType
{
	None,
	Single,
	Coop,
	Pvp
};

class GameGlobals : public dae::Singleton<GameGlobals>
{
public:
	char* GetPlayerName(int user) { return (char*)PlayerNames[user]; }

private:
	char PlayerNames[2][4];
};

