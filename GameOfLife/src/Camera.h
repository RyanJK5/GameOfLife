#ifndef __Camera_h__
#define __Camera_h__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Graphics2D.h"

namespace gol
{
	class Camera
	{
	public:
		float Zoom = 1.f;
		glm::vec2 Center = {};
	public:
		Camera() = default;
		Camera(float zoom, glm::vec2 center) : Zoom(zoom), Center(center) { }
	public:
		glm::mat4 OrthographicProjection(Size2 viewSize) const;
	};
}

#endif