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
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}

	ProjectionType GetProjectionType() const { return m_ProjectionType; }
	void SetProjectionType(int newProjectionType)
	{
		m_ProjectionType = (ProjectionType)newProjectionType;
		RecalculateProjection();
	}

	float GetOrthographicSize() const { return m_OrthoSize; }
	void SetOrthographicSize(float newSize)
	{
		m_OrthoSize = newSize;
		RecalculateProjection();
	}

	float GetOrthographicNearBound() const { return m_OrthoNear; }
	void SetOrthographicNearBound(float newNearBound)
	{
		m_OrthoNear = newNearBound;
		RecalculateProjection();
	}

	float GetOrthographicFarBound() const { return m_OrthoFar; }
	void SetOrthographicFarBound(float newFarBound)
	{
		m_OrthoFar = newFarBound;
		RecalculateProjection();
	}

	float GetPerspectiveFOVy() const { return m_PerspectiveFOVy; }
	void SetPerspectiveFOVy(float newFOVyRadians)
	{
		m_PerspectiveFOVy = newFOVyRadians;
		RecalculateProjection();
	}

	float GetPerspectiveNearBound() const { return m_PerspectiveNear; }
	void SetPerspectiveNearBound(float newNearBound)
	{
		m_PerspectiveNear = newNearBound;
		RecalculateProjection();
	}

	float GetPerspectiveFarBound() const { return m_PerspectiveFar; }
	void SetPerspectiveFarBound(float newFarBound)
	{
		m_PerspectiveFar = newFarBound;
		RecalculateProjection();
	}

	const glm::mat4& Camera::GetProjectionMatrix() const
	{
		return m_ProjectionMatrix;
	}

private:
	void RecalculateProjection();

private:
	// Defaults to an orthographic camera
	ProjectionType m_ProjectionType = ProjectionType::Orthographic;

	// Orthographic camera properties
	float m_OrthoSize = 10.0f;
	float m_OrthoNear = -10.0f;
	float m_OrthoFar = 10.0f;

	// Perspective camera properties
	float m_PerspectiveFOVy = glm::radians(45.0f);
	float m_PerspectiveNear = 0.01f;
	float m_PerspectiveFar = 100.0f;

	float m_AspectRatio = 0.0f;

	// Note this camera class only requires the projection matrix (since the camera pos/rot/scale is a part of the camera entity's transform)
	glm::mat4 m_ProjectionMatrix = glm::mat4(0);

};