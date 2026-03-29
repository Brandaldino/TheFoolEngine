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
	private:
		void DrawEntityNode(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};
}
