#include "Camera.h"

#include <glm\gtc\matrix_transform.hpp>

Camera::Camera()
{
	RecalculateProjection();
}

void Camera::SetOrthographic(float size, float nearClipBound, float farClipBound)
{
	m_ProjectionType = ProjectionType::Orthographic;

	m_OrthoSize = size;
	m_OrthoNear = nearClipBound;
	m_OrthoFar = farClipBound;

	RecalculateProjection();
}

void Camera::SetPerspective(float verticalFOVyRadians, float nearClipBound, float farClipBound)
{
	m_ProjectionType = ProjectionType::Perspective;

	m_PerspectiveFOVy = verticalFOVyRadians;
	m_PerspectiveNear = nearClipBound;
	m_PerspectiveFar = farClipBound;

	RecalculateProjection();
}

void Camera::RecalculateProjection()
{
	if (m_ProjectionType == ProjectionType::Perspective)
	{
		m_ProjectionMatrix = glm::perspective(m_PerspectiveFOVy, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
	}
	else if (m_ProjectionType == ProjectionType::Orthographic)
	{
		float orthoRightBound = m_AspectRatio * m_OrthoSize * 0.5f;
		float orthoLeftBound = -orthoRightBound;
		float orthoTopBound = m_OrthoSize * 0.5f;
		float orthoBottomBound = -orthoTopBound;
		m_ProjectionMatrix = glm::ortho(orthoLeftBound, orthoRightBound, orthoTopBound, orthoBottomBound, m_OrthoNear, m_OrthoFar);
	}
}
