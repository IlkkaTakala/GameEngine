#pragma once
#include <string>
#include <vector>
#include "Grid.h"
#include "glm/glm.hpp"

class PlayerComponent;
namespace dae {
	class Scene;
	class GameObject;
	typedef unsigned int User;
}

void makeDisplay(PlayerComponent* player, dae::Scene& scene);
dae::GameObject* makePlayer(dae::User user, dae::Scene& scene, float speed, int x, int y);
void makeGold(int x, int y);
void makeEnemy(int x, int y);
void makeEmerald(int x, int y);
void makeGoldBag(int x, int y);
void makeClearer(int x, int y, const std::vector<Direction>& path);
void makeSpawner(int x, int y);

struct LevelData
{
	glm::ivec2 playerStart;
	glm::ivec2 playerStart2;
	int emeraldCount;
	int nogginCount;
};
LevelData LoadLevel(const std::string& path);
void SaveLevel(const std::string& path);