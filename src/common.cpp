#include "common.hpp"

#include <glm/gtc/matrix_transform.hpp>

// Note, we could also use the functions from GLM but we write the transformations here to show the uderlying math
void Transform::scale(vec3 scale)
{
	matrix = glm::scale(matrix, scale);
}

void Transform::rotate(float radians, glm::vec3 axis)
{
	matrix = glm::rotate(matrix, radians, axis);
}

void Transform::translate(vec3 offset)
{
	matrix = glm::translate(matrix, offset);
}
