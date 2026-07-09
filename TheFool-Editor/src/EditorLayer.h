#pragma once

#include "TheFoolEngine.h"

#include "Panels/SceneHierarchyPanel.h"


namespace TheFoolEngine
{
    enum class ViewportModel
    {
        Mode2D,
        Mode3D
    };

	class EditorLayer : public TheFoolEngine::Layer
	{

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

	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(TimeStep ts) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
	private:
		// Temp
		Ref<VertexArray> m_SquareVA;
		Ref<Shader> m_FlatColorShader;
		Ref<FrameBuffer> m_FrameBuffer;

		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_MainCamera;
		Entity m_SecondCamera;

		bool m_PrimaryCamera = false;

		Ref<Texture2D> m_CheckerboardTexture;

		bool m_ViewportFocused = false, m_ViewportHovered = false;

		glm::vec2 m_ViewportSize = { 0.0f ,0.0f };

		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

        // === PBR TEST
        Ref<PBRModel> m_PBRModel;
        PerspectiveCameraController m_PerspectiveCameraController;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;

        std::vector<DebugLight> m_DebugLights;

        bool m_Is3DMode = true;
	};
}

