#include "Camera.h"

#include <glm\gtc\matrix_transform.hpp>

Camera::Camera()
{
	RecalculateViewMatrix();
	RecalculateProjectionMatrix();
}

void Camera::SetOrthographic(float size, float nearClipBound, float farClipBound)
{
	m_ProjectionType = ProjectionType::Orthographic;

	m_OrthoSize = size;
	m_OrthoNear = nearClipBound;
	m_OrthoFar = farClipBound;

	RecalculateProjectionMatrix();
}

void Camera::SetPerspective(float verticalFOVyRadians, float nearClipBound, float farClipBound)
{
	m_ProjectionType = ProjectionType::Perspective;

	m_PerspectiveFOVy = verticalFOVyRadians;
	m_PerspectiveNear = nearClipBound;
	m_PerspectiveFar = farClipBound;

	RecalculateProjectionMatrix();
}

void Camera::RecalculateProjectionMatrix()
{
	// TODO
	/*if (m_ProjectionType == ProjectionType::Perspective)
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
	}*/

	// Hardcoded orthographic camera for now (taken from template)
	float left = 0;
	float right = m_ViewportWidth;
	float top = 0;
	float bottom = m_ViewportHeight;
	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	m_ProjectionMatrix = glm::mat3{ { sx, 0.f, 0.f },{ 0.f, sy, 0.f },{ tx, ty, 1.f } };
}

void Camera::RecalculateViewMatrix()
{
	m_ViewMatrix = glm::inverse(glm::mat3{ { 1.f, 0.f, 0.f },{ 0.f, 1.f, 0.f },{ m_Position.x, m_Position.y, 1.f } });
}
