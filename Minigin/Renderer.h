#pragma once
#include <SDL.h>
#include <functional>
#include "Singleton.h"
#include <map>
#include "glm/glm.hpp"

namespace dae
{
	class Texture2D;
	/**
	 * Simple RAII wrapper for the SDL renderer
	 */
	class Renderer final : public Singleton<Renderer>
	{
		SDL_Renderer* m_renderer{};
		SDL_Window* m_window{};
		SDL_Color m_clearColor{};	

		std::multimap<int, std::function<void(void)>> m_renderSystems;
		std::vector<std::function<void(void)>> m_imGuiSystems;

	public:
		void Init(SDL_Window* window);
		void Render() const;
		void Destroy();
		glm::ivec2 GetWindowSize();

		void RenderTexture(const Texture2D& texture, float x, float y, bool center = false, float angle = 0.f, bool flip = false) const;
		void RenderTexture(const Texture2D& texture, float x, float y, float width, float height, bool center = false, float angle = 0.f, bool flip = false) const;
		void RenderTextureTiling(const Texture2D& texture, int x, int y, int width, int height) const;

		SDL_Renderer* GetSDLRenderer() const;

		template <class T>
		constexpr bool MakeRenderable(int prio) {
			AddRenderSubsystem(prio, []() {
				for (auto& o : T::ObjectList()) {
					if (o.IsValid() && o.IsActive())
						o.Render();
				}
			});
			return true;
		}

		const SDL_Color& GetBackgroundColor() const { return m_clearColor; }
		void SetBackgroundColor(const SDL_Color& color) { m_clearColor = color; }

	private:
		void AddRenderSubsystem(int prio, const std::function<void(void)>& system) {
			m_renderSystems.emplace(prio, system);
		}
	};
}

