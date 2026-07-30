#pragma once

#include "TheFoolEngine.h"

#include "Panels/SceneHierarchyPanel.h"


namespace TheFoolEngine
{
	class EditorLayer : public TheFoolEngine::Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(TimeStep ts) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;

        void PickEntity(const Ray::RayData& ray);
    private:
        void ImportModel();
        void ImportSkybox();
	private:
		Ref<FrameBuffer> m_HDRFrameBuffer;
		Ref<FrameBuffer> m_LDRFrameBuffer;

		Ref<FrameBuffer> m_BloomFBO_A;
		Ref<FrameBuffer> m_BloomFBO_B;
		Ref<Shader> m_BloomExtractShader;
		Ref<Shader> m_BloomBlurShader;
		Ref<Shader> m_BloomCombineShader;

        Ref<VertexArray> m_OutlineVAO;
        Ref<VertexBuffer> m_OutlineVBO;
        Ref<Shader> m_FlatShader;

		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_MainCamera;
		Entity m_SecondCamera;

		bool m_PrimaryCamera = false;

		bool m_ViewportFocused = false, m_ViewportHovered = false;

		glm::vec2 m_ViewportSize = { 0.0f ,0.0f };

        // === PBR TEST ===============================================
        Ref<PBRModel> m_PBRModel;
        PerspectiveCameraController m_PerspectiveCameraController;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;

        bool m_Is3DMode = true;

		Ref<Shader> m_ToneMappingShader;
		// ============================================================
	};
}

