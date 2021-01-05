#pragma once

#include <std_include.h>

/// <summary>
/// Represents a shooted ray from a certain point in a certain direction
/// </summary>
struct Ray
{
private:
	glm::vec3 _position;
	glm::vec3 _direction;

public:
	Ray(const Ray& ray) : _position(ray._position), _direction(ray._direction)
	{

	}

	void operator=(const Ray& ray) {
		_position = ray._position;
		_direction = ray._direction;
	}

	Ray& operator=(Ray&& move) noexcept = default;
	Ray(Ray&& ray) = default;

	Ray(const glm::vec3& position, const glm::vec3& direction)
	{
		_position = position;
		_direction = glm::normalize(direction);
	}

	Ray(glm::vec3&& position, glm::vec3&& direction)
	{
		_position = std::move(position);
		_direction = glm::normalize(std::move(direction));
	}

	/// <summary>
	/// Point in world coordinates from which the ray is shooted
	/// </summary>
	const glm::vec3& Position() const { return _position; }
	/// <summary>
	/// Normalized Direction of the shooted ray
	/// </summary>
	const glm::vec3& Direction() const { return _direction; }
};