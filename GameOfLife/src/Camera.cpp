#include "Camera.h"

void gol::Camera::ZoomBy(Vec2F screenPos, const RectF& viewBounds, float zoom)
{
    Zoom *= 1.f + zoom;
    Center += (ScreenToWorldPos(screenPos, viewBounds) - Center) * zoom;
}

glm::vec2 gol::Camera::ScreenToWorldPos(Vec2F pos, const Rect& viewBounds) const
{
    Vec2F cursor = ImGui::GetMousePos();

    glm::vec2 vec = {
        (cursor.X - viewBounds.X) ,
        (cursor.Y - viewBounds.Y) };
    glm::vec2 center = { viewBounds.Width / 2, viewBounds.Height / 2 };
    vec -= center;
    vec /= Zoom;
    vec += Center;
    return vec;
}

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
