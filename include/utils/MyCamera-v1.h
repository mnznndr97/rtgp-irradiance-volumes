#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

enum MOVEMENT
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;

const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVITY = 0.05f;

class Camera
{
private:
    glm::vec3 _position;
    glm::vec3 _front;
    glm::vec3 _worldFront;
    glm::vec3 _up;
    glm::vec3 _worldUp;
    glm::vec3 _right;

    GLfloat _yaw;
    GLfloat _pitch;
    GLfloat _movementSpeed;
    GLfloat _mouseSensitivity;

    GLboolean _onGround;

    void UpdateCameraVector()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        front.y = sin(glm::radians(_pitch));
        front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));

        _worldFront = _front = glm::normalize(front);
        if(_onGround) _worldFront.y = 0.0f;

        _right = glm::normalize(glm::cross(_front, _worldUp));
        _up = glm::normalize(glm::cross(_front, _right));
    }

public:
    Camera(glm::vec3 position, GLboolean onGround) : _position(position), _yaw(YAW), _pitch(PITCH), _movementSpeed(SPEED), _mouseSensitivity(SENSITIVITY), _onGround(onGround)
    {
        _worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        UpdateCameraVector();
    }

    void ProcessKeyboard(MOVEMENT direction, GLfloat deltaTime)
    {
        GLfloat velocity = _movementSpeed * deltaTime;
        if (direction == FORWARD)
            _position += (_onGround ? _worldFront : _front) * velocity;
        else if (direction == BACKWARD)
            _position -= (_onGround ? _worldFront : _front) * velocity;
        else if (direction == RIGHT)
            _position += _right * velocity;
        else if (direction == LEFT)
            _position -= _right * velocity;
    }

    void ProcessMouseMovement(GLfloat xOffset, GLfloat yOffset, GLboolean constrainPitch = true)
    {
        xOffset += _mouseSensitivity;
        yOffset += _mouseSensitivity;

        _yaw += xOffset;
        _pitch += yOffset;

        if (constrainPitch)
        {
            if (_pitch > 89.0f)
                _pitch = 89.0f;
            if (_pitch < -89.0f)
                _pitch = -89.0f;
        }

        UpdateCameraVector();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(_position, _position + _front, _up);
    }
};
