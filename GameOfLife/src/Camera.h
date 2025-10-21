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
		float Zoom = 0.0f;
		Vec2F Center = {};
	public:
		Camera() = default;
		Camera(float zoom, Vec2F center) : Zoom(zoom), Center(center) { }
	public:
		glm::mat4 OrthographicProjection(Size2 viewSize) const;
	};
}

#endif