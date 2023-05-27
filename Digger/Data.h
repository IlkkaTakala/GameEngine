#pragma once

constexpr int TileSize{ 45 };
constexpr int SpriteSize{ int(TileSize * 0.9) };

namespace Events
{
	constexpr int PlayerDeath = 100;
	constexpr int PlayerFinalScore = 130;
	constexpr int PlayerScoreGained = 101;
	constexpr int ScoreEmerald = 121;
	constexpr int ScoreGold = 122;
	constexpr int ScoreEnemy = 123;
	constexpr int GoldBagCrush = 102;
}
