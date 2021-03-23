#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

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
	if (m_AspectRatio <= 0.0f)
		return;

	if (m_ProjectionType == ProjectionType::Perspective)
	{
		m_ProjectionMatrix = glm::perspective(m_PerspectiveFOVy, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
		// bug fix for now
		m_ProjectionMatrix[1][1] = -m_ProjectionMatrix[1][1];
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

void Camera::RecalculateViewMatrix()
{
	// Create the View Matrix by inverting the camera position/rotation transformations 
	glm::mat4 transformationMatrix = glm::mat4(1.0f);
	// Translation
	transformationMatrix = glm::translate(transformationMatrix, m_Position);
	// Rotation (z axis)
	transformationMatrix = glm::rotate(transformationMatrix, glm::radians(m_RotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	// Rotation (y axis)
	transformationMatrix = glm::rotate(transformationMatrix, glm::radians(m_RotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	// Now take the inverse to get the View Matrix
	m_ViewMatrix = glm::inverse(transformationMatrix);

}
