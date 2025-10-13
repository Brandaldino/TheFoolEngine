#include "tfpch.h"
#include "PerspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace TheFoolEngine {

	PerspectiveCamera::PerspectiveCamera(float fovDegrees, float aspectRatio, float nearClip, float farClip)
		:m_FovDegrees(fovDegrees), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
	{

		m_ViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
		m_ProjectionMatrix = glm::perspective(glm::radians(fovDegrees), aspectRatio, nearClip, farClip);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::SetPerspective(float fovDegrees, float aspectRatio, float nearClip, float farClip)
	{
		m_FovDegrees = fovDegrees;
		m_AspectRatio = aspectRatio;
		m_NearClip = nearClip;
		m_FarClip = farClip;

		RecalculateFrustumMatrix();
	}

	void PerspectiveCamera::SetAspectRatio(float aspectRatio)
	{
		m_AspectRatio = aspectRatio;
		RecalculateFrustumMatrix();
	}

	void PerspectiveCamera::RecalculateForward()
	{
		glm::vec3 newForward;
		newForward.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		newForward.y = sin(glm::radians(m_Pitch));
		newForward.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		m_Forward = glm::normalize(newForward);

		m_Right = glm::normalize(glm::cross(m_Forward, glm::vec3(0.0f, 1.0f, 0.0f)));
		m_Up = glm::normalize(glm::cross(m_Right, m_Forward));

		RecalculateFrustumMatrix();
	}

	void PerspectiveCamera::RecalculateFrustumMatrix()
	{
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
		m_ProjectionMatrix = glm::perspective(glm::radians(m_FovDegrees), m_AspectRatio, m_NearClip, m_FarClip);

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}
