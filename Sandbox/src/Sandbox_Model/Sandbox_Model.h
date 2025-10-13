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
	

	// ---------------- test function -------------
	void SphereTest();

private:
	TFE::PerspectiveCameraController m_CameraController;

	TFE::Ref<TFE::Shader> m_Shader;
	// TFE::Ref<TFE::Texture3D> m_Texture;

	TFE::Ref<TFE::ModelImporter> m_Model_1;
	// TFE::Ref<TFE::ModelImporter> m_Model_2;

	std::vector<TFE::Ref<TFE::MeshData>> m_MeshDatas;
	uint32_t m_DefaultMaterialHash;

	// Temp Test
	TFE::Ref<TFE::Texture2D> m_Container;
	TFE::Ref<TFE::Texture2D> m_ContainerSpecular;
	TFE::Ref<TFE::VertexArray> m_VertexArray;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
	glm::vec3 m_LightPos;

	uint32_t m_LastDrawCallsCount = 0;
	uint32_t m_LastMeshCount = 0;
};
