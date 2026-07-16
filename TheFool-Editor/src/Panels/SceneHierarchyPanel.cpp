#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>

#include "TheFoolEngine/Scene/Components.h"

#include <glm/gtc/type_ptr.hpp>

namespace TheFoolEngine
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.each([&](auto entityID)
			{
				Entity entity{ entityID, m_Context.get() };
				DrawEntityNode(entity);
			}
		);

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		if (opened)
		{
            // Tag
            DrawTagComponent(entity);

            // Transform
            if (entity.HasComponent<TransformComponent>())
                DrawTransformComponent(entity);

            // SpriteRenderer
            if (entity.HasComponent<SpriteRendererComponent>())
                DrawSpriteRendererComponent(entity);

            // Camera
            if (entity.HasComponent<CameraComponent>())
                DrawCameraComponent(entity);

            // Light
            if (entity.HasComponent<LightComponent>())
                DrawLightComponent(entity);

            // PBRModel
            if (entity.HasComponent<PBRModelComponent>())
                DrawPBRModelComponent(entity);

			ImGui::TreePop();
		}
	}

    void SceneHierarchyPanel::DrawTagComponent(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer) - 1);
        if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
            tag = std::string(buffer);
    }

    void SceneHierarchyPanel::DrawTransformComponent(Entity entity)
    {
        if (ImGui::CollapsingHeader("Transform"))
        {
            auto& transform = entity.GetComponent<TransformComponent>().Transform;
            ImGui::DragFloat4("Row 0", glm::value_ptr(transform[0]), 0.05f);
            ImGui::DragFloat4("Row 1", glm::value_ptr(transform[1]), 0.05f);
            ImGui::DragFloat4("Row 2", glm::value_ptr(transform[2]), 0.05f);
            ImGui::DragFloat4("Row 3", glm::value_ptr(transform[3]), 0.05f);
        }
    }

    void SceneHierarchyPanel::DrawSpriteRendererComponent(Entity entity)
    {
        if (ImGui::CollapsingHeader("Sprite Renderer"))
        {
            auto& color = entity.GetComponent<SpriteRendererComponent>().Color;
            ImGui::ColorEdit4("Color", glm::value_ptr(color));
        }
    }

    void SceneHierarchyPanel::DrawCameraComponent(Entity entity)
    {
        if (ImGui::CollapsingHeader("Camera"))
        {
            auto& cc = entity.GetComponent<CameraComponent>();
            ImGui::Checkbox("Primary", &cc.Primary);

            float orthoSize = cc.Camera.GetOrthographicSize();
            if (ImGui::DragFloat("Ortho Size", &orthoSize))
                cc.Camera.SetOrthographicSize(orthoSize);
        }
    }

    void SceneHierarchyPanel::DrawLightComponent(Entity entity)
    {
        if (ImGui::CollapsingHeader("Light"))
        {
            auto& lc = entity.GetComponent<LightComponent>();

            const char* types[] = { "Directional", "Point", "Spot" };
            ImGui::Combo("Type", &lc.Type, types, 3);

            if (lc.Type == 0 || lc.Type == 2)
                ImGui::DragFloat3("Direction", glm::value_ptr(lc.Direction), 0.05f);
            if (lc.Type == 1 || lc.Type == 2)
                ImGui::DragFloat3("Position", glm::value_ptr(lc.Position), 0.1f);

            ImGui::ColorEdit3("Color", glm::value_ptr(lc.Color));
            ImGui::DragFloat("Intensity", &lc.Intensity, 0.1f, 0.0f, 100.0f);

            if (lc.Type == 1 || lc.Type == 2)
                ImGui::DragFloat("Range", &lc.Range, 0.1f, 0.0f, 100.0f);
            if (lc.Type == 2)
            {
                ImGui::SliderAngle("Inner Angle", &lc.InnerAngle, 0.0f, 90.0f);
                ImGui::SliderAngle("Outer Angle", &lc.OuterAngle, 0.0f, 90.0f);
            }
        }
    }

    void SceneHierarchyPanel::DrawPBRModelComponent(Entity entity)
    {
        if (ImGui::CollapsingHeader("PBR Model"))
        {
            ImGui::Text("Model: %s", entity.GetComponent<PBRModelComponent>().Model->GetPath().u8string().c_str());
        }
    }
}
