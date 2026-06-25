#pragma once

#include "PerspectiveCamera.h"
#include "TheFoolEngine/Core/TimeStep.h"

#include "TheFoolEngine/Events/ApplicationEvents.h"
#include "TheFoolEngine/Events/MouseEvents.h"

namespace TheFoolEngine{

	class PerspectiveCameraController {
	public:
		PerspectiveCameraController(float aspectRatio);

		void OnUpdate(TimeStep ts);
		void OnEvent(Event& e);

		void OnResize(float width, float height);
		// void OnFocus(int botton);
		void AdjustCameraAngles(float x, float y); // adjust by mouse move

		PerspectiveCamera& GetCamera() { return m_Camera; }
		const PerspectiveCamera& GetCamera() const { return m_Camera; }

		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }
	private:
		bool OnWindowResized(WindowResizeEvent& e);
		// bool OnMouseButtonRelesed(MouseButtonEvent& e);
		// bool OnMouseMoved(MouseMovedEvent& e);
	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		PerspectiveCamera m_Camera;

		glm::vec3 m_CameraPosition;
		glm::vec3 m_CameraForward;
		glm::vec3 m_CameraRight;
		glm::vec3 m_CameraUp;

		float m_Pitch, m_Yaw, m_Roll;

		float m_CameraTranslationSpeed = 3.0f;

		glm::vec2 m_MouseLastPosition = { -1.0f,-1.0f };
		bool m_ShouldResetMouseReference = true;
		float m_MouseSensitivity = 0.1f;
	};

}
