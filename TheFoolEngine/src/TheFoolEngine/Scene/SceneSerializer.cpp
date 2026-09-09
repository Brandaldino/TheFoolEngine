#include "tfpch.h"
#include "SceneSerializer.h"

#include "Scene.h"
#include "Components.h"
#include "JsonGlm.h"

#include "../Renderer/PBRRenderer.h"

#include <nlohmann/json.hpp>

using Json = nlohmann::json;

namespace TheFoolEngine
{

    void SceneSerializer::Serialize(Ref<Scene> scene, const std::string& filepath)
    {
        Json sceneJson;
        sceneJson["Scene"]["Name"] = "DefaultScene";

        // environment
        sceneJson["Scene"]["Environment"] = Json::object();

        // Iterate over entities
        auto view = scene->m_Registry.view<TagComponent>();
        for (auto entity : view)
        {
            Json entityJson;

            // Tag
            entityJson["TagComponent"]["Tag"] = view.get<TagComponent>(entity).Tag;

            // Transform
            if (scene->m_Registry.all_of<TransformComponent>(entity))
            {
                auto& tc = scene->m_Registry.get<TransformComponent>(entity);
                entityJson["TransformComponent"]["Transform"] = tc.Transform;
            }

            // Light
            if (scene->m_Registry.all_of<LightComponent>(entity))
            {
                auto& lc = scene->m_Registry.get<LightComponent>(entity);
                entityJson["LightComponent"]["Type"] = lc.Type;
                entityJson["LightComponent"]["Direction"] = lc.Direction;
                entityJson["LightComponent"]["Color"] = lc.Color;
                entityJson["LightComponent"]["Intensity"] = lc.Intensity;
                entityJson["LightComponent"]["Position"] = lc.Position;
                entityJson["LightComponent"]["Range"] = lc.Range;
                entityJson["LightComponent"]["InnerAngle"] = lc.InnerAngle;
                entityJson["LightComponent"]["OuterAngle"] = lc.OuterAngle;
            }

            // PBRModel
            if (scene->m_Registry.all_of<PBRModelComponent>(entity))
            {
                auto& mc = scene->m_Registry.get<PBRModelComponent>(entity);
                // entityJson["PBRModelComponent"]["Name"] = mc.Model->GetName();
                entityJson["PBRModelComponent"]["FilePath"] = mc.Model->GetPath().string();
            }

            // SpriteRenderer
            if (scene->m_Registry.all_of<SpriteRendererComponent>(entity))
            {
                auto& src = scene->m_Registry.get<SpriteRendererComponent>(entity);
                entityJson["SpriteRendererComponent"]["Color"] = src.Color;
            }

            // Camera
            if (scene->m_Registry.all_of<CameraComponent>(entity))
            {
                auto& cc = scene->m_Registry.get<CameraComponent>(entity);
                entityJson["CameraComponent"]["SceneCamera"]["ProjectionType"] = static_cast<int>(cc.Camera.GetProjectionType());
                entityJson["CameraComponent"]["SceneCamera"]["PerspectiveFOV"] = cc.Camera.GetPerspectiveFOV();
                entityJson["CameraComponent"]["SceneCamera"]["PerspectiveNear"] = cc.Camera.GetPerspectiveNear();
                entityJson["CameraComponent"]["SceneCamera"]["PerspectiveFar"] = cc.Camera.GetPerspectiveFar();
                entityJson["CameraComponent"]["SceneCamera"]["OrthographicSize"] = cc.Camera.GetOrthographicSize();
                entityJson["CameraComponent"]["SceneCamera"]["OrthographicNear"] = cc.Camera.GetOrthographicNear();
                entityJson["CameraComponent"]["SceneCamera"]["OrthographicFar"] = cc.Camera.GetOrthographicFar();
                entityJson["CameraComponent"]["Primary"] = cc.Primary;
                entityJson["CameraComponent"]["FixedAspectRatio"] = cc.FixedAspectRatio;
            }

            sceneJson["Scene"]["Entity"].push_back(entityJson);
        }

        std::ofstream out(filepath);
        out << sceneJson.dump(6);
    }

    void SceneSerializer::Deserialize(Ref<Scene> scene, const std::string& filepath)
    {
        if (!std::filesystem::exists(filepath))
        {
            TF_CORE_ERROR("Scene file does not exist: {0}", filepath);
            return;
        }

        std::ifstream in(filepath);
        if (!in)
        {
            TF_CORE_ERROR("Failed to open scene file: {0}", filepath);
            return;
        }

        Json sceneJson;
        try 
        {
            in >> sceneJson;
        }
        catch (const Json::parse_error& e)
        {
            TF_CORE_ERROR("Failed to open scene file {0}: {1}", filepath, e.what());
            return;
        }

        scene->m_Registry.clear();

        for (auto& entityJson : sceneJson["Scene"]["Entity"])
        {
            std::string name = entityJson["TagComponent"]["Tag"].get<std::string>();
            Entity entity = scene->CreateEntity(name);

            if (entityJson.contains("TransformComponent"))
                entity.GetComponent<TransformComponent>().Transform = entityJson["TransformComponent"]["Transform"].get<glm::mat4>();

            if (entityJson.contains("LightComponent"))
            {
                auto& jl = entityJson["LightComponent"];
                entity.AddComponent<LightComponent>(LightComponent{
                    jl["Type"].get<int>(),
                    jl["Position"].get<glm::vec3>(),
                    jl["Direction"].get<glm::vec3>(),
                    jl["Color"].get<glm::vec3>(),
                    jl["Intensity"].get<float>(),
                    jl["Range"].get<float>(),
                    jl["InnerAngle"].get<float>(),
                    jl["OuterAngle"].get<float>()
                    });
            }

            if (entityJson.contains("PBRModelComponent"))
            {
                auto model = CreateRef<PBRModel>();
                std::filesystem::path path = entityJson["PBRModelComponent"]["FilePath"].get<std::string>();
                model->Import(path);
                PBRRenderer::DefaultTextureFill(model);
                model->UpLoad();

                entity.AddComponent<PBRModelComponent>(model);
            }

            if (entityJson.contains("SpriteRendererComponent"))
                entity.AddComponent<SpriteRendererComponent>(entityJson["SpriteRendererComponent"]["Color"].get<glm::vec4>());

            if (entityJson.contains("CameraComponent"))
            {
                auto& jc = entityJson["CameraComponent"]["SceneCamera"];
                CameraComponent cc;
                cc.Camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(
                    jc.value("ProjectionType", 0)));
                cc.Camera.SetPerspective(
                    jc.value("PerspectiveFOV", 0.785f),
                    jc.value("PerspectiveNear", 0.01f),
                    jc.value("PerspectiveFar", 1000.0f));
                cc.Camera.SetOrthographic(
                    jc.value("OrthographicSize", 10.0f),
                    jc.value("OrthographicNear", -1.0f),
                    jc.value("OrthographicFar", 1.0f));
                cc.Primary = jc.value("Primary", true);
                cc.FixedAspectRatio = jc.value("FixedAspectRatio", false);

                entity.AddComponent<CameraComponent>(cc);
            }
        }

        TF_INFO("Lights after load: {0}", (int)scene->m_Registry.view<LightComponent>().size());
    }

}