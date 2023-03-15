#include <SDL.h>

#if _DEBUG
// ReSharper disable once CppUnusedIncludeDirective
#if __has_include(<vld.h>)
#include <vld.h>
#endif
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "GameObject.h"
#include "Renderer.h"
#include "TextComponent.h"
#include "FPSComponent.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"
#include "PointRotatorComponent.h"
#include "CacheSpeedTestComponent.h"
#include "Time.h"
#include "imgui.h"
#include "imgui_plot.h"

#include <fstream>

float Time = 0.f;

void load()
{
	using namespace dae;

	Renderer::GetInstance().AddRenderSubsystem([]() {
		for (auto& o : SpriteComponent::__object_list()) {
			if (o.IsValid())
				o.Render();
		}
		for (auto& o : TextComponent::__object_list()) {
			if (o.IsValid())
				o.Render();
		}
	});

	auto& scene = SceneManager::GetInstance().CreateScene("Demo");
	auto go3 = new GameObject();
	auto font = ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto title = CreateComponent<TextComponent>(go3);
	auto trans = CreateComponent<TransformComponent>(go3);
	title->Init("Programming 4 Assignment", font);
	trans->SetPosition(80, 20, 0);
	scene.Add(go3);

	auto trash1 = new GameObject();
	auto trasher = CreateComponent<CacheSpeedTestComponent>(trash1);
	trasher->SetTestType(0);
	scene.Add(trash1);

	const float points[11]{ 1.f, 2.f, 4.f, 8.f, 16.f, 32.f, 64.f, 128.f, 256.f, 512.f, 1024.f };

	Renderer::GetInstance().AddImGuiSystem([ref = trasher->GetPermanentReference(), points]() {
		auto ptr = ref.Get<CacheSpeedTestComponent>();
		ImGuiWindowFlags window_flags = 0;
		if (!ImGui::Begin("Exercise 1", nullptr, window_flags))
		{
			ImGui::End();
			return;
		}

		ImGui::InputInt("Samples", ptr->GetStepsPtr());
		ImGui::Separator();
		ImGui::Spacing();

		if (ptr->IsCalculating()) {
			ImGui::Text("Waiting for data...");
		}
		else {
			if (ImGui::Button("Trash the cache")) {
				ptr->Recalculate();
			}
		}
		if (ptr->HasData()) {
			auto& data = ptr->GetData();
			ImGui::PlotConfig conf;
			conf.frame_size = { 300, 200 };
			conf.values.ys = data.data();
			conf.values.xs = points;
			conf.values.count = (int)data.size();
			conf.scale.min = -100;
			conf.scale.max = data[0] * 1.2f;
			conf.grid_x.show = true;
			conf.grid_y.show = true;
			conf.grid_y.size = 1000.f;
			conf.tooltip.show = true;
			conf.tooltip.format = "x=%.2f, y=%.2f";
			ImGui::Plot("", conf);
		}

		ImGui::End();
	});

	auto trash2 = new GameObject();
	auto trasher2 = CreateComponent<CacheSpeedTestComponent>(trash2);
	trasher2->SetTestType(1);
	scene.Add(trash2);

	auto trash3 = new GameObject();
	auto trasher3 = CreateComponent<CacheSpeedTestComponent>(trash3);
	trasher3->SetTestType(2);
	scene.Add(trash3);

	Renderer::GetInstance().AddImGuiSystem([
			ref1 = trash2->GetComponent<CacheSpeedTestComponent>()->GetPermanentReference(), 
			ref2 = trash3->GetComponent<CacheSpeedTestComponent>()->GetPermanentReference(), 
			points
		]() {
		auto ptr1 = ref1.Get<CacheSpeedTestComponent>();
		auto ptr2 = ref2.Get<CacheSpeedTestComponent>();
		ImGuiWindowFlags window_flags = 0;
		if (!ImGui::Begin("Exercise 2", nullptr, window_flags))
		{
			ImGui::End();
			return;
		}

		ImGui::InputInt("Samples", ptr1->GetStepsPtr());
		ptr2->SetSteps(*ptr1->GetStepsPtr());
		ImGui::Separator();
		ImGui::Spacing();

		if (ptr1->IsCalculating()) {
			ImGui::Text("Waiting for data...");
		}
		else {
			if (ImGui::Button("Trash the cache with GameObject3D")) {
				ptr1->Recalculate();
			}
		}
		if (ptr1->HasData()) {
			auto& data = ptr1->GetData();
			ImGui::PlotConfig conf;
			conf.frame_size = { 300, 200 };
			conf.values.ys = data.data();
			conf.values.xs = points;
			conf.values.count = (int)data.size();
			conf.scale.min = -100;
			conf.scale.max = data[0] * 1.2f;
			conf.grid_x.show = true;
			conf.grid_y.show = true;
			conf.grid_y.size = 1000.f;
			conf.tooltip.show = true;
			conf.tooltip.format = "x=%.2f, y=%.2f";
			conf.values.color = ImGui::ColorConvertFloat4ToU32({1, 0, 0, 1});
			ImGui::Plot("", conf);
		}

		if (ptr2->IsCalculating()) {
			ImGui::Text("Waiting for data...");
		}
		else {
			if (ImGui::Button("Trash the cache with GameObject3DAlt")) {
				ptr2->Recalculate();
			}
		}
		if (ptr2->HasData()) {
			auto& data = ptr2->GetData();
			ImGui::PlotConfig conf;
			conf.frame_size = { 300, 200 };
			conf.values.ys = data.data();
			conf.values.xs = points;
			conf.values.count = (int)data.size();
			conf.scale.min = -100;
			conf.scale.max = data[0] * 1.2f;
			conf.grid_x.show = true;
			conf.grid_y.show = true;
			conf.grid_y.size = 1000.f;
			conf.tooltip.show = true;
			conf.tooltip.format = "x=%.2f, y=%.2f";
			conf.values.color = ImGui::ColorConvertFloat4ToU32({ 0, 1, 0, 1 });
			ImGui::Plot("", conf);
		}

		ImGui::Text("Combined");

		if (ptr1->HasData() && ptr2->HasData()) {
			float* y_data[2] = { ptr1->HasData() ? ptr1->GetData().data() : nullptr,  ptr2->HasData() ? ptr2->GetData().data() : nullptr };
			auto& data1 = ptr1->GetData();
			auto& data2 = ptr2->GetData();
			ImGui::PlotConfig conf;
			conf.frame_size = { 300, 200 };
			conf.values.ys_list = (const float**)y_data;
			conf.values.ys_count = 2;
			conf.values.xs = points;
			conf.values.count = int(data1.size() > data2.size() ? data1.size() : data2.size());
			conf.scale.min = -100;
			float max = 0.f;
			if (max < data1[0]) max = data1[0] * 1.2f;
			if (max < data2[0]) max = data2[0] * 1.2f;
			conf.scale.max = max;
			conf.grid_x.show = true;
			conf.grid_y.show = true;
			conf.grid_y.size = 1000.f;
			conf.tooltip.show = true;
			ImU32 color[2] = { ImGui::ColorConvertFloat4ToU32({1, 0, 0, 1}), ImGui::ColorConvertFloat4ToU32({ 0, 1, 0, 1 }) };
			conf.values.colors = color;
			ImGui::Plot("", conf);
		}

		ImGui::End();
		});
}

struct Vector
{
	float x, y, z;
};

int main(int, char*[]) {
	dae::Minigin engine("../Data/");
	engine.Run(load);
	return 0;
}