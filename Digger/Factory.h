#pragma once

class PlayerComponent;
namespace dae {
	class Scene;
	typedef unsigned int User;
}

void makeDisplay(PlayerComponent* player, dae::Scene& scene);
void makePlayer(dae::User user, dae::Scene& scene, float speed);
void makeGold(int x, int y);
void makeEnemy(int x, int y);
void makeEmerald();
void makeGoldBag();
void makeClearer();
void makeSpawner(int x, int y);