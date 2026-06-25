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
	TFE::PerspectiveCameraController m_CameraController;

    TFE::PBRRenderProxy m_PBRProxy;

	glm::vec3 m_LightPos;
    float m_TotalTime = 0.0f;
};
