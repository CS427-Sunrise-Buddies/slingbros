#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	enum class ProjectionType { Perspective = 0, Orthographic = 1 };

public:
	Camera();
	virtual ~Camera() = default;

	void SetOrthographic(float size, float nearClipBound, float farClipBound);
	void SetPerspective(float verticalFOVyRadians, float nearClipBound, float farClipBound);

	void SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjectionMatrix();
	}

	float GetRotationZ() const { return m_RotationZ; }
	void SetRotationZ(float degrees)
	{
		m_RotationZ = degrees;
		RecalculateViewMatrix();
	}

	glm::vec3 GetPosition() const { return m_Position; }
	void SetPosition(glm::vec3 position)
	{
		m_Position = position;
		RecalculateViewMatrix();
	}

	ProjectionType GetProjectionType() const { return m_ProjectionType; }
	void SetProjectionType(int newProjectionType)
	{
		m_ProjectionType = (ProjectionType)newProjectionType;
		RecalculateProjectionMatrix();
	}

	float GetOrthographicSize() const { return m_OrthoSize; }
	void SetOrthographicSize(float newSize)
	{
		m_OrthoSize = newSize;
		RecalculateProjectionMatrix();
	}

	float GetOrthographicNearBound() const { return m_OrthoNear; }
	void SetOrthographicNearBound(float newNearBound)
	{
		m_OrthoNear = newNearBound;
		RecalculateProjectionMatrix();
	}

	float GetOrthographicFarBound() const { return m_OrthoFar; }
	void SetOrthographicFarBound(float newFarBound)
	{
		m_OrthoFar = newFarBound;
		RecalculateProjectionMatrix();
	}

	float GetPerspectiveFOVy() const { return m_PerspectiveFOVy; }
	void SetPerspectiveFOVy(float newFOVyRadians)
	{
		m_PerspectiveFOVy = newFOVyRadians;
		RecalculateProjectionMatrix();
	}

	float GetPerspectiveNearBound() const { return m_PerspectiveNear; }
	void SetPerspectiveNearBound(float newNearBound)
	{
		m_PerspectiveNear = newNearBound;
		RecalculateProjectionMatrix();
	}

	float GetPerspectiveFarBound() const { return m_PerspectiveFar; }
	void SetPerspectiveFarBound(float newFarBound)
	{
		m_PerspectiveFar = newFarBound;
		RecalculateProjectionMatrix();
	}

	const glm::mat4& GetProjectionMatrix() const
	{
		return m_ProjectionMatrix;
	}
	const glm::mat4& GetViewMatrix() const
	{
		return m_ViewMatrix;
	}

private:
	void RecalculateProjectionMatrix();
	void RecalculateViewMatrix();

private:
	// Defaults to an orthographic camera
	ProjectionType m_ProjectionType = ProjectionType::Orthographic;

	// Orthographic camera properties
	float m_OrthoSize = 1000.0f;
	float m_OrthoNear = -10.0f;
	float m_OrthoFar = 10.0f;

	// Perspective camera properties
	float m_PerspectiveFOVy = glm::radians(45.0f);
	float m_PerspectiveNear = 0.01f;
	float m_PerspectiveFar = 100.0f;

	float m_AspectRatio = 0.0f;
	uint32_t m_ViewportWidth, m_ViewportHeight;

	glm::mat4 m_ProjectionMatrix = glm::mat4(1); 

	float m_RotationZ = 0; // units are in degrees
	glm::vec3 m_Position = glm::vec3(0, 0, 0);
	glm::mat4 m_ViewMatrix = glm::mat4(1);

};