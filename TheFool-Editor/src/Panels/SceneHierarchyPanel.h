#pragma once

#include "TheFoolEngine/Core/Base.h"
#include "TheFoolEngine/Core/Log.h"
#include "TheFoolEngine/Scene/Scene.h"
#include "TheFoolEngine/Scene/Entity.h"

namespace TheFoolEngine
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);
		void SetContext(const Ref<Scene>& scene);
		void OnImGuiRender();
        Entity GetSelectionContext() const { return m_SelectionContext; };
        void SetSelectionContext(Entity entity) { m_SelectionContext = entity; };
	private:
		void DrawEntityNode(Entity entity);
        void DrawTagComponent(Entity entity);
        void DrawTransformComponent(Entity entity);
        void DrawSpriteRendererComponent(Entity entity);
        void DrawCameraComponent(Entity entity);
        void DrawLightComponent(Entity entity);
        void DrawPBRModelComponent(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};
}
