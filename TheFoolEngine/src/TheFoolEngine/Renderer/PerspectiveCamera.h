#pragma once

#include <glm/glm.hpp>

namespace TheFoolEngine {

	class PerspectiveCamera {
	public:
		PerspectiveCamera(float fovDegrees = 45.0f, float aspectRatio = 1280.0f / 720.0f, float nearClip = 0.1f, float farClip = 100.0f);

		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::vec3& GetForward() const { return m_Forward; }
		const glm::vec3& GetRight() const { return m_Right; }
		const glm::vec3& GetUp() const { return m_Up; }

		void SetPerspective(float fovDegrees, float aspectRatio, float nearClip, float farClip);
		void SetAspectRatio(float aspectRatio);

		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateFrustumMatrix(); }

		void SetForward(const glm::vec3& forward) { m_Forward= forward; RecalculateForward(); }
		void SetRight(const glm::vec3& right) { m_Right = right; RecalculateForward(); }
		void SetUp(const glm::vec3& up) { m_Up= up; RecalculateForward(); }

		float GetPitch() const { return m_Pitch; }
		void SetPitch(float pitch) { m_Pitch = pitch; RecalculateForward(); }

		float GetYaw() const { return m_Yaw; }
		void SetYaw(float yaw) { m_Yaw = yaw; RecalculateForward(); }

		float GetRoll() const { return m_Roll; }
		void SetRoll(float roll) { m_Roll = roll; RecalculateForward(); }

		const glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4 GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4 GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
	private:
		void RecalculateForward();
		void RecalculateFrustumMatrix();
	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0.0f , 0.0f , 15.0f };

		float m_Pitch = 0.0f, m_Yaw = -90.0f, m_Roll = 0.0f;

		glm::vec3 m_Forward = { 0.0f, 0.0f , -1.0f };
		glm::vec3 m_Right = { 1.0f, 0.0f ,0.0f };
		glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };

		float m_FovDegrees, m_AspectRatio, m_NearClip, m_FarClip;
	};
}