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
	constexpr int BonusPickup = 124;
	constexpr int GoldBagCrush = 102;

	constexpr int EnemyDestroy = 141;
	constexpr int EmeraldDestroy = 142;

	constexpr int SkipLevel = 161;
	constexpr int EnemySpawn = 162;
}
