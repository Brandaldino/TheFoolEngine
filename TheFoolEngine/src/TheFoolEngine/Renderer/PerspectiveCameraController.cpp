#include "tfpch.h"
#include "PerspectiveCameraController.h"

#include "TheFoolEngine/Core/Input.h"
#include "TheFoolEngine/Core/KeyCodes.h"
#include "TheFoolEngine/Core/MouseCodes.h"

// TODO: 相机的移动应按照朝向时的方位移动

namespace TheFoolEngine {

	PerspectiveCameraController::PerspectiveCameraController(float aspectRatio)
		:m_AspectRatio(aspectRatio),m_Camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f)
	{
		m_CameraPosition = m_Camera.GetPosition();
		m_CameraForward = m_Camera.GetForward();
		m_CameraRight = m_Camera.GetRight();
		m_CameraUp = m_Camera.GetUp();

		m_Pitch = m_Camera.GetPitch();
		m_Yaw = m_Camera.GetYaw();
		m_Roll = m_Camera.GetRoll();
	}

	void PerspectiveCameraController::OnUpdate(TimeStep ts) 
	{
		float velocity = m_CameraTranslationSpeed * ts;

		if (Input::IsKeyPressed(Key::W))
			m_CameraPosition -= m_CameraForward * velocity;
		else if (Input::IsKeyPressed(Key::S))
			m_CameraPosition += m_CameraForward * velocity;

		if (Input::IsKeyPressed(Key::A))
			m_CameraPosition -= m_CameraRight * velocity;
		else if (Input::IsKeyPressed(Key::D))
			m_CameraPosition += m_CameraRight * velocity;

		if (Input::IsKeyPressed(Key::Q))
			m_CameraPosition -= m_CameraUp * velocity;
		else if (Input::IsKeyPressed(Key::E))
			m_CameraPosition += m_CameraUp * velocity;

		if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
			AdjustCameraAngles(Input::GetMouseX(), Input::GetMouseY());
		else
			m_ShouldResetMouseReference = true;

		m_Camera.SetPosition(m_CameraPosition);
	}

	void PerspectiveCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(TF_BIND_EVENT_FN(PerspectiveCameraController::OnWindowResized));
		// dispatcher.Dispatch<MouseButtonEvent>(TF_BIND_EVENT_FN(PerspectiveCameraController::OnWindowResized));
		// dispatcher.Dispatch<MouseMovedEvent>(TF_BIND_EVENT_FN(PerspectiveCameraController::OnMouseMoved));
	}

	void PerspectiveCameraController::OnResize(float width, float height)
	{
		m_AspectRatio = width / height;

		m_Camera.SetAspectRatio(m_AspectRatio);
	}

	void PerspectiveCameraController::AdjustCameraAngles(float x, float y)
	{
		if (m_ShouldResetMouseReference)
		{
			m_MouseLastPosition.x = x;
			m_MouseLastPosition.y = y;
			m_ShouldResetMouseReference = false;
			return;
		}

		float xoffset = x - m_MouseLastPosition.x;
		float yoffset = m_MouseLastPosition.y - y;
		m_MouseLastPosition.x = x;
		m_MouseLastPosition.y = y;

		const float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		m_Yaw += xoffset;
		m_Pitch += yoffset;

		m_Camera.SetPitch(m_Pitch);
		m_Camera.SetYaw(m_Yaw);

		glm::mat4 inverseView = glm::inverse(m_Camera.GetViewMatrix());
		m_CameraForward = glm::normalize(glm::vec3(inverseView[2]));
		m_CameraRight = glm::normalize(glm::vec3(inverseView[0]));
		m_CameraUp = glm::normalize(glm::vec3(inverseView[1]));

		m_Camera.SetForward(m_CameraForward);
		m_Camera.SetRight(m_CameraRight);
		m_Camera.SetUp(m_CameraUp);
	}

	bool PerspectiveCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		OnResize(e.GetWidth(), e.GetHeight());
		return false;
	}

	//bool PerspectiveCameraController::OnMouseMoved(MouseMovedEvent& e)
	//{
	//	AdjustCameraAngles(e.GetX(), e.GetY());
	//	return false;
	//}
}
