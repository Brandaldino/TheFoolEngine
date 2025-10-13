#pragma once

#include "TheFoolEngine.h"

class Sandbox2D : public TheFoolEngine::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(TheFoolEngine::TimeStep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(TheFoolEngine::Event& e) override;
private:
	TheFoolEngine::OrthographicCameraController m_CameraController;

	// Temp
	TheFoolEngine::Ref<TheFoolEngine::VertexArray> m_SquareVA;
	TheFoolEngine::Ref<TheFoolEngine::Shader> m_FlatColorShader;

	TheFoolEngine::Ref<TheFoolEngine::Texture2D> m_CheckerboardTexture;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};

