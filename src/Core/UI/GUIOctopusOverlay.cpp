#include "GUIOctopusOverlay.h"
#include "Outrospection.h"
#include "Core/UI/GUIScene.h"


void eyeClick(UIButton& button, int mouseButton)
{
    // play a random Eye_Poke sound
    int index = rand() / (RAND_MAX / 3);
    Outrospection::get().audioManager.play("Eye_Poke_" + std::to_string(index));

    button.setAnimation("blink");
    Util::doLater([&button]() { button.setAnimation("default"); }, 100);

    auto pokedEye = (Eye)button.name[8];
    
    if (mouseButton == GLFW_MOUSE_BUTTON_RIGHT)
    {
        Outrospection::get().setEye(pokedEye);
    }
    else // do the action
    {
        ((GUIScene*)Outrospection::get().scene)->doControl(pokedEye);
    }
}

void eyeHover(UIButton& button, int)
{
    auto scene = (GUIScene*)Outrospection::get().scene;

    auto pokedEye = (Eye)button.name[8];
    scene->ghostInputQueue.clear();
    scene->ghostSprite.hidden = false;
    scene->doGhostControl(pokedEye);
}

void eyeUnhover(UIButton& button, int)
{
    auto scene = (GUIScene*)Outrospection::get().scene;

    scene->ghostInputQueue.clear();
    scene->ghostSprite.hidden = true;
}

void reset(UIButton&, int)
{
    Util::doLater([]
    {
        LOG_INFO("Resetting...");
        auto& o = Outrospection::get();
        ((GUIScene*)o.scene)->reset();
    }, 100);
}

void undo(UIButton&, int)
{
    Util::doLater([]
    {
        auto& o = Outrospection::get();
        ((GUIScene*)o.scene)->tryUndo();
    }, 100);
}

void muteOrUnmute(UIButton& muteButton, int)
{
    static bool muted = false;

    if(muted) // unmute
    {
        muted = false;
        Outrospection::get().audioManager.setGlobalVolume(1.0);
        muteButton.setAnimation("mute");
    } else
    {
        muted = true;
        Outrospection::get().audioManager.setGlobalVolume(0.0);
        muteButton.setAnimation("unmute");
    }
}

GUIOctopusOverlay::GUIOctopusOverlay() : GUILayer("Octopus Overlay", false),
                                         background("overlay/background", GL_LINEAR, UITransform(0, 0, 1920, 1080)),
                                         octopus("octopus", animatedTexture({ "UI/overlay/", "octopus" },
                                             2, 21, GL_LINEAR), UITransform(0, 0, 1920, 1080))
{
    auto &bCircle = buttons.emplace_back(std::make_unique<UIButton>("eyes/eyeCircle0", GL_LINEAR, UITransform(0, 0, 1920, 1080),
                                            Bounds(UITransform(1401, 108, 134, 0), BoundsShape::Circle), eyeClick));
    bCircle->addAnimation("blink", animatedTexture({"UI/eyes/", "eyeCircle"}, 1, 5, GL_LINEAR));
    bCircle->onHover = eyeHover;
    bCircle->onUnhover = eyeUnhover;

    auto& bSquare = buttons.emplace_back(std::make_unique<UIButton>("eyes/eyeSquare0", GL_LINEAR, UITransform(0, 0, 1920, 1080),
                                            Bounds(UITransform(1690, 54, 134, 0), BoundsShape::Circle), eyeClick));
    bSquare->addAnimation("blink", animatedTexture({"UI/eyes/", "eyeSquare"}, 1, 5, GL_LINEAR));
    bSquare->onHover = eyeHover;
    bSquare->onUnhover = eyeUnhover;

    auto& bTriangle = buttons.emplace_back(std::make_unique<UIButton>("eyes/eyeTriangle0", GL_LINEAR, UITransform(0, 0, 1920, 1080),
                                            Bounds(UITransform(1786, 300, 134, 0), BoundsShape::Circle), eyeClick));
    bTriangle->addAnimation("blink", animatedTexture({"UI/eyes/", "eyeTriangle"}, 1, 5, GL_LINEAR));
    bTriangle->onHover = eyeHover;
    bTriangle->onUnhover = eyeUnhover;

    // gap of 26 px between buttons
    buttons.emplace_back(std::make_unique<UIButton>("reset", GL_NEAREST, UITransform(1811, 970, 96, 96), Bounds(), reset));
    buttons.emplace_back(std::make_unique<UIButton>("undo", GL_NEAREST, UITransform(1691, 970, 96, 96), Bounds(), undo));

    auto& bMute = buttons.emplace_back(std::make_unique<UIButton>("mute", GL_NEAREST, UITransform(1571, 970, 96, 96), Bounds(), muteOrUnmute));
    bMute->addAnimation("unmute", simpleTexture({"UI/", "unmute"}, GL_NEAREST));
}
void GUIOctopusOverlay::tick()
{
    background.tick();
    octopus.tick();

    for (auto& button : buttons)
    {
        button->tick();
    }
}

void GUIOctopusOverlay::draw() const
{
    background.draw(Outrospection::get().spriteShader, Outrospection::get().glyphShader);
    octopus.draw(Outrospection::get().spriteShader, Outrospection::get().glyphShader);

    for (auto& button : buttons)
    {
        button->draw(Outrospection::get().spriteShader, Outrospection::get().glyphShader);
    }
}
