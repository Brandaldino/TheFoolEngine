#pragma once

#include <glm/glm.hpp>

#include "SceneCamera.h"
#include "ScriptableEntity.h"
#include "../Importer/PBRModel.h"

namespace TheFoolEngine{

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {
		}
	};

	struct TransformComponent
	{
		glm::mat4 Transform{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::mat4& transform)
			: Transform(transform) {
		}

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f , 1.0f , 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {
		}
	};

	struct CameraComponent
	{
		TheFoolEngine::SceneCamera Camera; 
		bool Primary = true;
		bool FixedAspectRatio = false;	// lock aspect ratio

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

    struct PBRModelComponent
    {
        Ref<PBRModel> Model;
    };

    struct LightComponent
    {
        int Type = 0; // Direction = 0, Point = 1, Spot = 2
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Direction = { -1.0f, -1.0f, -1.0f };
        glm::vec3 Color = { 1.0f,1.0f, 1.0f };
        float Intensity = 1.0f;
        float Range = 10.0f;
        float InnerAngle = glm::radians(15.0f);
        float OuterAngle = glm::radians(30.0f);
    };

}
