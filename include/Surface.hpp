#pragma once

#include <std_include.h>
#include <optional>
#include <Ray.hpp>

class RadianceP {
public:
	/// <summary>
	/// 
	/// </summary>
	/// <remarks>Also enabled for implicit conversion</remarks>
	RadianceP(const glm::vec3& rad) : Value(rad) {

	}

	const std::string Name = "Radiance";
	const glm::vec3 Value;
};

class Surface {
private:
	std::optional<RadianceP> _radiance;
public:
	const std::optional<RadianceP>& GetRadiance() const {
		return _radiance;
	}

	void SetRadiance(const RadianceP& radiance) {
		_radiance.emplace(radiance);
	}
};

struct LazyReflection {
private:
	glm::vec3 _normal;
	glm::vec3 _direction;
public:
	LazyReflection(const glm::vec3& n, const glm::vec3& d)
		: _normal(n), _direction(d)
	{

	}

	LazyReflection(glm::vec3&& n, glm::vec3&& d)
		: _normal(n), _direction(d)
	{
	}

	LazyReflection(const LazyReflection& other)
		: _normal(other._normal), _direction(other._direction)
	{

	}

	LazyReflection(LazyReflection&& other) = default;
	LazyReflection& operator=(LazyReflection&& other) noexcept = default;

	glm::vec3 operator()() const {
		return glm::normalize(glm::reflect(_normal, _direction));
	}
};

class RayHit {
private:
	bool _isHit;
	Surface* _surface;
	glm::vec3 _hitPoint;
	GLfloat _distance;
	LazyReflection _reflection;

public:
	RayHit() : _isHit(false), _surface(nullptr), _hitPoint(glm::vec3(0.0f)), _distance(0.0f), _reflection(glm::vec3(0.0f), glm::vec3(0.0f)) {

	}

	RayHit(Surface* surface, const glm::vec3& hp, GLfloat d, const LazyReflection& r) :
		_isHit(true), _surface(nullptr), _hitPoint(hp), _distance(d), _reflection(r) {

	}

	RayHit(Surface* surface, const glm::vec3& hp, GLfloat d, LazyReflection&& r) :
		_isHit(true), _surface(surface),
		_hitPoint(hp), _distance(d),
		_reflection(std::move(r)) {

		assert(_distance > 0.0f);
	}

	RayHit(Surface* surface, glm::vec3&& hp, GLfloat d, LazyReflection&& r) :
		_isHit(true), _surface(surface),
		_hitPoint(hp), _distance(d),
		_reflection(std::move(r)) {

		assert(_distance > 0.0f);
	}

	bool IsHit() const { return _isHit; }
	GLfloat Distance() const { return _distance; }
	const glm::vec3& HittingPoint() const { return _hitPoint; }
	const LazyReflection& Reflection() const { return _reflection; }
	Surface* Surface() const { return _surface; }
};

