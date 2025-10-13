#pragma once

#include "entt.hpp"
#include "TheFoolEngine/Core/TimeStep.h"

namespace TheFoolEngine{

	class Entity;

	class Scene 
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());

		void OnUpdate(const TimeStep& ts);
	private:
		entt::registry m_Registry;

		friend class Entity;
	};

}
