#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <nlohmann/json.hpp>

namespace nlohmann
{
    template<>
    struct adl_serializer<glm::vec3>
    {
        static void to_json(json& j, const glm::vec3& v)
        {
            j = json{ v.x, v.y, v.z };
        }
        static void from_json(const json& j, glm::vec3& v)
        {
            v = glm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
        }
    };

    template<>
    struct adl_serializer<glm::vec4>
    {
        static void to_json(json& j, const glm::vec4& v)
        {
            j = json{ v.x, v.y, v.z, v.w };
        }
        static void from_json(const json& j, glm::vec4& v)
        {
            v = glm::vec4(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>());
        }
    };

    template<>
    struct adl_serializer<glm::mat4>
    {
        static void to_json(json& j, const glm::mat4& m)
        {
            const float* p = glm::value_ptr(m);
            j = json{ p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
                      p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15] };
        }
        static void from_json(const json& j, glm::mat4& m)
        {
            std::array<float, 16> a;
            for (int i = 0; i < 16; ++i)
                a[i] = j[i].get<float>();
            m = glm::make_mat4(a.data());
        }
    };
}