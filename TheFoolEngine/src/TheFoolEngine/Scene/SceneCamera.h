#pragma once

#include "TheFoolEngine/Renderer/Camera.h"

namespace TheFoolEngine
{

	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		enum class ProjectionType 
        { 
            Perspective = 0, 
            Orthographic = 1 
        };

		void SetPerspective(float fov, float nearClip, float farClip);
		void SetOrthographic(float size, float nearClip, float farClip);
		void SetViewportSize(uint32_t width, uint32_t height);
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		float GetPerspectiveFOV() const { return m_PerspectiveFOV; }
		float GetOrthographicSize() const { return m_OrthographicSize; }

		void SetPerspectiveFOV(float fov) { m_PerspectiveFOV = fov; RecalculateProjection(); }
		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }

	private:
		void RecalculateProjection();

		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		float m_PerspectiveFOV = glm::radians(45.0f);
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_AspectRatio = 0.0f;
	};
}
