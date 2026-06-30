#pragma once

#include <TheFoolEngine.h>

namespace TFE = TheFoolEngine;

class Sandbox_Model : public TFE::Layer {
public:
	Sandbox_Model();
	virtual ~Sandbox_Model() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(TFE::TimeStep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(TFE::Event& event) override;
private:
    struct DebugLight
    {
        int Type = 0;               // 0=directional, 1=point, 2=spot
        glm::vec3 Position = { 5.0f, 5.0f, 5.0f };
        glm::vec3 Direction = { -1.0f, -1.0f, -1.0f };
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        float Intensity = 1.5f;
        float Range = 20.0f;
        float InnerAngle = glm::radians(15.0f);
        float OuterAngle = glm::radians(25.0f);
    };

	TFE::PerspectiveCameraController m_CameraController;

    TFE::PBRRenderProxy m_PBRProxy;

    std::vector<DebugLight> m_DebugLights;
};
