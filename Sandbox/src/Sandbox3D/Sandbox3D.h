#pragma once

#include <TheFoolEngine.h>

namespace TFE = TheFoolEngine;

class Sandbox3D : public TFE::Layer {
public:
	Sandbox3D();
	virtual ~Sandbox3D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(TFE::TimeStep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(TFE::Event& event) override;
	

private:
	TFE::PerspectiveCameraController m_CameraController;

	TFE::Ref<TFE::Shader> m_Shader;

	// Temp Test
	TFE::Ref<TFE::Texture2D> m_Container;
	TFE::Ref<TFE::Texture2D> m_ContainerSpecular;
	TFE::Ref<TFE::VertexArray> m_VertexArray;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
	glm::vec3 m_LightPos;
};
