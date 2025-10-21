#include "Camera.h"

glm::mat4 gol::Camera::OrthographicProjection(Size2 viewSize) const
{
	return glm::ortho(0.f, viewSize.Width / Zoom, viewSize.Height / Zoom, 0.f, -1.f, 1.f);
}
