#include "BaseComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "Grid.h"

using namespace dae;

ComponentRef<Grid> makeGrid(const GridData& data)
{
	auto go = new GameObject();

	auto grid = CreateComponent<Grid>(go);
	auto trans = CreateComponent<TransformComponent>(go);
	grid->Init(data);
	trans->SetPosition({ data.x, data.y, 0.f });


	SceneManager::GetInstance().GetCurrentScene()->Add(go);
	return grid;
}

void Grid::Render()
{
	if (!Texture) return;
	glm::vec3 pos{ 0 };
	if (auto t = GetOwner()->GetComponent<TransformComponent>()) {
		pos = t->GetPosition();
	}
	
	Renderer::GetInstance().RenderTexture(*Texture, pos.x, pos.y, false);
	Renderer::GetInstance().RenderTexture(*Overlay, pos.x, pos.y, false);
}

void Grid::Init(const GridData& data)
{
	Data = data;
	int w = Data.cells_x * Data.size;
	int h = Data.cells_y * Data.size;
	auto renderer = Renderer::GetInstance().GetSDLRenderer();

	SDL_Texture* texTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_TARGET, w, h);
	Texture = std::make_shared<Texture2D>(texTarget);
	SDL_Texture* texTarget2 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET, w, h);
	SDL_SetTextureBlendMode(texTarget2, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, texTarget2);
	SDL_RenderClear(renderer);
	Overlay = std::make_shared<Texture2D>(texTarget2);

	EatTexs.push_back(ResourceManager::GetInstance().LoadTexture("VUBLOB.png"));
	EatTexs.push_back(ResourceManager::GetInstance().LoadTexture("VRBLOB.png"));
	EatTexs.push_back(ResourceManager::GetInstance().LoadTexture("VDBLOB.png"));
	EatTexs.push_back(ResourceManager::GetInstance().LoadTexture("VLBLOB.png"));
	EatTexs.push_back(ResourceManager::GetInstance().LoadTexture("TILE.png"));

	auto Back = ResourceManager::GetInstance().LoadTexture("VBACK1.gif");
	SDL_SetRenderTarget(renderer, Texture->GetSDLTexture());
	SDL_RenderClear(renderer);
	Renderer::GetInstance().RenderTextureTiling(*Back, 0, 0, w, h);
	//Detach the texture
	SDL_SetRenderTarget(renderer, NULL);

	Cells.resize(data.cells_x * data.cells_y);
}

void Grid::ClearCell(Direction dir, int x, int y)
{
	static int opposite[] = {
		2,
		3,
		0,
		1
	};
	int idx = y * Data.cells_x + x;
	if (idx >= 0 && idx < Cells.size()) {
		Cells[idx].Cleared = true;
		if (dir != Direction::None) {
			glm::ivec2 old = glm::ivec2{ x, y } - Opposites[(int)dir];
			int prevIdx = old.y * Data.cells_x + old.x;
			if (prevIdx >= 0 && prevIdx < Cells.size()) {
				Cells[prevIdx].Tunnels[(int)dir] = &Cells[idx];
				Cells[idx].Tunnels[opposite[(int)dir]] = &Cells[prevIdx];
			}
		}
	}
}

void Grid::MoveOut(GameObject* o, int x, int y)
{
	int idx = y * Data.cells_x + x;
	if (idx >= 0 && idx < Cells.size()) {
		std::erase(Cells[idx].Objects, o);
	}
}

void Grid::MoveInto(GameObject* o, int x, int y)
{
	int idx = y * Data.cells_x + x;
	if (idx >= 0 && idx < Cells.size()) {
		Cells[idx].Objects.push_back(o);
	}
}

glm::vec3 Grid::GetCellCenter(int x, int y)
{
	glm::vec3 o{ Data.x, Data.y, 0.f };
	glm::vec3 size{ Data.size, Data.size, 0.f };

	return o + size * 0.5f + glm::vec3{ x, y, 0.f } *size;
}

CellData* Grid::GetCellInDirection(Direction dir, int x, int y)
{
	y += Opposites[(int)dir].y;
	x += Opposites[(int)dir].x;
	if (x < 0 || x >= Data.cells_x) return nullptr;
	if (y < 0 || y >= Data.cells_y) return nullptr;
	int idx = y * Data.cells_x + x;
	if (idx >= 0 && idx < Cells.size()) {
		return &Cells[idx];
	}
	return nullptr;
}

void Grid::Eat(Direction dir, int x, int y)
{
	auto renderer = Renderer::GetInstance().GetSDLRenderer();
	SDL_SetRenderTarget(renderer, Overlay->GetSDLTexture());
	SDL_SetTextureBlendMode(EatTexs[(int)dir]->GetSDLTexture(), SDL_BLENDMODE_BLEND);
	Renderer::GetInstance().RenderTexture(*EatTexs[(int)dir], (float)x - Data.x, (float)y - Data.y, (float)Data.size * 1.1f, (float)Data.size * 1.1f, true);
	SDL_SetRenderTarget(renderer, NULL);
}

void Grid::EatCell(int x, int y)
{
	ClearCell(Direction::None, x, y);
	auto cent = GetCellCenter(x, y);
	auto renderer = Renderer::GetInstance().GetSDLRenderer();
	SDL_SetRenderTarget(renderer, Overlay->GetSDLTexture());
	SDL_SetTextureBlendMode(EatTexs[4]->GetSDLTexture(), SDL_BLENDMODE_BLEND);
	Renderer::GetInstance().RenderTexture(*EatTexs[4], cent.x - Data.x, cent.y - Data.y, (float)Data.size * 1.0f, (float)Data.size * 1.0f, true);
	SDL_SetRenderTarget(renderer, NULL);
}
