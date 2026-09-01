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
        TextureHandle m_HDRHandle;
        TextureHandle m_LDRHandle;
        TextureHandle m_BloomAHandle;
        TextureHandle m_BloomBHandle;
        TextureHandle m_BloomCHandle;

        Ref<Shader> m_BloomExtractShader;
		Ref<Shader> m_BloomBlurShader;
		Ref<Shader> m_BloomCombineShader;
        Ref<Shader> m_ToneMappingShader;

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

        RenderGraph m_RenderGraph;
        Scope<MainPass> m_MainPass;
        Scope<ShadowPass> m_ShadowPass;
        Scope<PointShadowPass> m_PointShadowPass;

        // Post-rendering processing
        Scope<BloomExtractPass> m_BloomExtractPass;
        Scope<BloomBlurPass> m_BloomBurPassH;
        Scope<BloomBlurPass> m_BloomBurPassV;
        Scope<BloomCombinePass> m_BloomCombinePass;
        Scope<ToneMappingPass> m_ToneMappingPass;

        // === PBR TEST ===============================================
        Ref<PBRModel> m_PBRModel;
        PerspectiveCameraController m_PerspectiveCameraController;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;

        bool m_Is3DMode = true;
        // === ShadowRenderer =========================================
        Ref<ShadowRenderer> m_ShadowRenderer;
		// ============================================================
	};
}

