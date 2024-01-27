#pragma once

#include "base.h"

namespace FF {

	class Camera {

	public:
		enum CAMERA_MOVE
		{
			MOVE_LEFT, 
			MOVE_RIGHT, 
			MOVE_FORWARD, 
			MOVE_BACKWARD
		};

	public:
		Camera() {
			_position = glm::vec3(1.0f);
			_front = glm::vec3(1.0f);
			_up = glm::vec3(1.0f);

			_pitch = 0.0f;
			_yaw = -90.0f;
			_sensitivity = 0.1f;

			_xpos = 0;
			_ypos = 0;

			_firstMove = true;
			
			_viewMatrix = glm::mat4(1.0f);
		}

		~Camera() {

		}

		void lookAt(glm::vec3 pos, glm::vec3 front, glm::vec3 up) {
			_position = pos;
			_front = front;
			_up = up;
			_viewMatrix = glm::lookAt(_position, _position + _front, _up);
		}

		void setPersipective(float angle, float ration, float near, float far) {
			_projectMatrix = glm::perspective(glm::radians(angle), ration, near, far);
		}

		void setSpeed(float speed) {
			_speed = speed;
		}

		void move(CAMERA_MOVE move) {
			switch (move)
			{
			case FF::Camera::MOVE_LEFT:
				_position -= glm::normalize(glm::cross(_front, _up)) * _speed;
				break;
			case FF::Camera::MOVE_RIGHT:
				_position += glm::normalize(glm::cross(_front, _up)) * _speed;
				break;
			case FF::Camera::MOVE_FORWARD:
				_position += _speed * _front;
				break;
			case FF::Camera::MOVE_BACKWARD:
				_position -= _speed * _front;
				break;
			default:
				break;
			}
			update();
		}

		void onMouseMove(double xpos, double ypos) {
			if (_firstMove) {
				_xpos = xpos;
				_ypos = ypos;
				_firstMove = false;
			}
			else {
				_pitch += -(ypos-_ypos) * _sensitivity;
				if (_pitch > 89.0f) {
					_pitch = 89.0f;
				}

				if (_pitch <= -89.f) {
					_pitch = -89.0f;
				}

				_yaw += (xpos-_xpos) * _sensitivity;

				_xpos = xpos;
				_ypos = ypos;

				_front.y = sin(glm::radians(_pitch));
				_front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
				_front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));

				_front = glm::normalize(_front);
			}
			update();
		}

		void setSensitivity(float sensitivity) {
			_sensitivity = sensitivity;
		}

		void update() {
			_viewMatrix = glm::lookAt(_position, _position + _front, _up);
		}



		glm::mat4 getViewMatrix() const {
			return _viewMatrix;
		}

		glm::mat4 getProjectionMatrix() const {
			return _projectMatrix;
		}


	private:
		glm::vec3 _position;
		glm::vec3 _front;
		glm::vec3 _up;
		float _speed;

		float _pitch;
		float _yaw;
		float _sensitivity;
		
		float _xpos;
		float _ypos;
		bool _firstMove;

		glm::mat4 _viewMatrix;
		glm::mat4 _projectMatrix;
	};

}