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
	private:
		Ref<FrameBuffer> m_FrameBuffer;

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
	};
}

