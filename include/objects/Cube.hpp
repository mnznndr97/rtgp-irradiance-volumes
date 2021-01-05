#pragma once

#include <std_include.h>
#include <objects/CubeWall.hpp>
#include <BCube.hpp>

class CCube : public SceneObject {
private:
	Wall* _leftWall;
	Wall* _rightWall;
	Wall* _backWall;
	Wall* _frontWall;
	Wall* _topWall;
	Wall* _bottomWall;

	Wall* _walls[6];
	BCube _objBoudingCube;
	BCube _objTransformedBoudingCube;

	virtual void OnTransformChanged() override {
		_objTransformedBoudingCube = _objBoudingCube >> GetTransform();
	};
public:
	CCube() :
		SceneObject(Shader("shaders/simple.vert", "shaders/simple.frag")),
		_leftWall(nullptr), _rightWall(nullptr), _backWall(nullptr),
		_frontWall(nullptr), _topWall(nullptr), _bottomWall(nullptr)
	{
		glm::vec3 grayWall = glm::vec3(0.4f);
		glm::vec3 leftWallVertices[] = {
			glm::vec3(-0.5f, -0.5f,  0.5f),
			glm::vec3(-0.5f,  0.5f,  0.5f),
			glm::vec3(-0.5f, -0.5f, -0.5f),
			glm::vec3(-0.5f,  0.5f, -0.5f) };

		_leftWall = new Wall(leftWallVertices, 4, _shader);
		_leftWall->GetSurface()->SetRadiance(glm::vec3(0.4f, 0.0f, 0.0f));

		glm::vec3 backVertices[] = {
			 glm::vec3(-0.5f, -0.5f, -0.5f),
			 glm::vec3(-0.5f,  0.5f, -0.5f),
			 glm::vec3(0.5f, -0.5f,  -0.5f),
			 glm::vec3(0.5f,  0.5f,  -0.5f)
		};
		_backWall = new Wall(backVertices, 4, _shader);
		_backWall->GetSurface()->SetRadiance(grayWall);

		glm::vec3 rightVertices[] = {
			 glm::vec3(0.5f, -0.5f, -0.5f),
			 glm::vec3(0.5f,  0.5f, -0.5f),
			 glm::vec3(0.5f, -0.5f,  0.5f),
			 glm::vec3(0.5f,  0.5f,  0.5f)
		};
		_rightWall = new Wall(rightVertices, 4, _shader);
		_rightWall->GetSurface()->SetRadiance(glm::vec3(0.0f, 0.0f, .7f));

		glm::vec3 frontVertices[] = {
			 glm::vec3(-0.5f, -0.5f, 0.5f),
			 glm::vec3(+0.5f, -0.5f, 0.5f),
			 glm::vec3(-0.5f,  0.5f, 0.5f),
			 glm::vec3(0.5f,  0.5f,  0.5f)
		};
		_frontWall = new Wall(frontVertices, 4, _shader);
		_frontWall->GetSurface()->SetRadiance(grayWall);

		glm::vec3 topVertices[] = {
			 glm::vec3(-0.5f,  0.5f,  0.5f),
			 glm::vec3(-0.5f, 0.5f, -0.5f),
			 glm::vec3(0.5f, 0.5f,  0.5f),
			 glm::vec3(0.5f, 0.5f, -0.5f)
		};
		_topWall = new Wall(topVertices, 4, _shader);
		_topWall->GetSurface()->SetRadiance(grayWall);

		glm::vec3 bottomVertices[] = {
			 glm::vec3(-0.5f, -0.5f,  0.5f),
			 glm::vec3(-0.5f, -0.5f, -0.5f),
			 glm::vec3(0.5f, -0.5f,  0.5f),
			 glm::vec3(0.5f, -0.5f, -0.5f)
		};
		_bottomWall = new Wall(bottomVertices, 4, _shader);
		_bottomWall->GetSurface()->SetRadiance(grayWall);

		_walls[0] = _leftWall;
		_walls[1] = _rightWall;
		_walls[2] = _backWall;
		_walls[3] = _topWall;
		_walls[4] = _bottomWall;
		_walls[5] = _frontWall;

		_objBoudingCube = BCube::FromMinMax(glm::vec3(-0.49f), glm::vec3(0.49f));
		SetScale(glm::vec3(15.5f));
	}

	virtual void SetScale(glm::vec3&& value) override {
		SetScale(value);
	}

	virtual void SetScale(glm::vec3& value) override {
		SceneObject::SetScale(value);
		for (int i = 0; i < 6; i++)
		{
			_walls[i]->SetScale(value);
		}
	}

	virtual RayHit IsHitByRay(const Ray& ray) const override {
		static const RayHit empty;
		for (int i = 0; i < 6; i++)
		{
			RayHit hit = _walls[i]->IsHitByRay(ray);
			if (hit.IsHit()) return hit;
		}
		// Nothing has been hit
		return empty;
	}

	virtual BCube& GetBoundingCube() override {
		return _objBoudingCube;
	}
	virtual BCube& GetTransformedBoundingCube() override {
		return _objTransformedBoudingCube;
	}

	virtual void Draw() override {
		_leftWall->Draw();
		_rightWall->Draw();
		_backWall->Draw();
		_topWall->Draw();
		_bottomWall->Draw();

		// ~ Cube is open ~
		//_frontWall->Draw();
	}

	virtual ~CCube() override {
		delete _leftWall;
		delete _rightWall;
		delete _backWall;
		delete _frontWall;
		delete _topWall;
		delete _bottomWall;
	}
};