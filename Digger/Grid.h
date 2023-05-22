#pragma once
#include "glm/glm.hpp"
#include "Renderer.h"
#include "GameObject.h"
#include "BaseComponent.h"

struct GridData
{
	int x, y;
	int cells_x, cells_y;
	int size;
};

struct CellData
{
	bool Cleared{ false };
	CellData* Tunnels[4]{ nullptr };
	std::vector<dae::GameObject*> Objects;
};

enum class Direction
{
	Up,
	Right,
	Down,
	Left,
	None
};


static glm::ivec2 Opposites[] = {
		{ 0, -1 },
		{ 1, 0 },
		{ 0, 1 },
		{ -1, 0 },
};

namespace dae {
	class Texture2D;
}
class Grid final : public dae::Component<Grid>
{
	SET_RENDER_PRIORITY(0)
public:

	void Render();

	void Init(const GridData& data);

	void MoveOut(dae::GameObject* o, int x, int y);
	void MoveInto(dae::GameObject* o, int x, int y);
	void ClearCell(Direction dir, int x, int y);
	glm::vec3 GetCellCenter(int x, int y);
	CellData* GetCellInDirection(Direction dir, int x, int y);
	CellData* GetCell(int x, int y);

	void Eat(Direction dir, int x, int y);
	void EatCell(int x, int y);

	GridData Data;
private:

	std::shared_ptr<dae::Texture2D> Texture;
	std::shared_ptr<dae::Texture2D> Overlay;
	std::vector<std::shared_ptr<dae::Texture2D>> EatTexs;

	std::vector<CellData> Cells;
};

dae::ComponentRef<Grid> makeGrid(const GridData& data);