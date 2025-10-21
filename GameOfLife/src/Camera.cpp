#include "Camera.h"

glm::mat4 gol::Camera::OrthographicProjection(Size2 viewSize) const
{
	return glm::ortho(
		Center.x - (viewSize.Width / 2.f)  / Zoom,
		Center.x + (viewSize.Width / 2.f)  / Zoom,
		Center.y + (viewSize.Height / 2.f) / Zoom,
		Center.y - (viewSize.Height / 2.f)  / Zoom, 
		-1.f, 1.f
	);
}
