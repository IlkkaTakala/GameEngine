#pragma once
#include <string>
#include <memory>
#include "SDL_pixels.h"
#include "BaseComponent.h"
#include "Renderer.h"

namespace dae
{
	class Font;
	class Texture2D;
	class TextComponent final : public Component<TextComponent>
	{
	public:
		void Tick(float delta) override;

		void Render() const;

		void SetText(const std::string& text);
		void SetColor(const SDL_Color& color);

		void Init(const std::string& text, std::shared_ptr<Font> font);

	private:
		bool m_needsUpdate{ true };
		std::string m_text;
		std::shared_ptr<Font> m_font;
		std::shared_ptr<Texture2D> m_textTexture;
		SDL_Color m_textColor{ 255,255,255 };
	};
}
