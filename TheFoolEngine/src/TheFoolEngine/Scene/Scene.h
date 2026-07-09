#pragma once

#include "entt.hpp"
#include "TheFoolEngine/Core/TimeStep.h"

namespace TheFoolEngine
{

	class Entity;

	class Scene 
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());

		void OnUpdate(const TimeStep& ts, bool render2D = true);
		void OnViewportResize(uint32_t width, uint32_t height);
	private:
		//template<typename T>
		//void OnComponentAdded(Entity entity, T& component)
		//{
		//	static_assert(sizeof(T) == 0);
		//}
	private:
		entt::registry m_Registry;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class EditorLayer;

        
	};

}
