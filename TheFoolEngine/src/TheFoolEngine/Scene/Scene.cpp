#include "tfpch.h"
#include "Scene.h"

#include "Components.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Entity.h"

namespace TheFoolEngine
{
	Scene::Scene()
	{
#if ENTT_EXAMPLE_CODE
		entt::entity entity = m_Registry.create();
		m_Registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));

		// m_Registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();

		//if (m_Registry.has<TransformComponent>(entity))
		//	TransformComponent& transform = m_Registry.get<TransformComponent>(entity);

		auto view = m_Registry.view<TransformComponent>();
		for (auto entity : view)
		{
			TransformComponent& transform = m_Registry.get<TransformComponent>(entity);
		}

		auto group = m_Registry.group<TransformComponent>();
		for (auto entity : group)
		{
			auto&[transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);
		}
#endif
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
        entity.AddComponent<IDComponent>();
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::OnUpdate(const TimeStep& ts)
	{
		// Update Scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
				{
					// TODO: Move to Scene::OnScenePlay
					if(!nsc.Instance)
					{
						nsc.Instance = nsc.InstantiateScript();
						nsc.Instance->m_Entity = Entity{ entity, this};
						nsc.Instance->OnCreate();
					}

					nsc.Instance->OnUpdate(ts);
				}
			);
		}
	}
	
	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
        if (width == 0 || height == 0)
            return;

		// Resize our non-FixedAspectRatio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}
	}

}
