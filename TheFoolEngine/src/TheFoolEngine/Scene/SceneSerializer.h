#pragma once

#include <string>

namespace TheFoolEngine
{
    class SceneSerializer
    {
        friend class Scene;
    public:
        static void Serialize(Ref<Scene> scene, const std::string& filepath);
        static void Deserialize(Ref<Scene> scene, const std::string& filepath);
    };
}