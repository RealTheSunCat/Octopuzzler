#include "UIButton.h"

#include <utility>

#include "Outrospection.h"

UIButton::UIButton(const std::string& _texName, const float posXPercent, const float posYPercent,
                   const float widthPercent,
                   const float heightPercent, Bounds bounds, ButtonCallback clickCallback)
	: UIComponent(_texName, posXPercent,
                 posYPercent,
                 widthPercent,
                 heightPercent),
	onClick(std::move(clickCallback)),
	buttonBounds(bounds)
{
}

UIButton::UIButton(const std::string& _name, SimpleTexture& tex, const float posXPercent, const float posYPercent,
    const float widthPercent, const float heightPercent,
    Bounds bounds, ButtonCallback clickCallback)
	: UIComponent(_name, tex,
	    glm::vec2(SCR_WIDTH * posXPercent, SCR_HEIGHT* posYPercent),
	    glm::vec2(SCR_WIDTH* widthPercent, SCR_HEIGHT* heightPercent)),
    onClick(std::move(clickCallback)),
	buttonBounds(bounds)
{
}

bool UIButton::isOnButton(const glm::vec2& point) const
{
    return buttonBounds.contains(point);
}

void UIButton::tick()
{
    glm::vec2 mousePos = Outrospection::get().lastMousePos;
    hovered = isOnButton(mousePos);
}
