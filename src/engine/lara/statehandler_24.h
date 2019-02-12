#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "loader/file/level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_24 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_24(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::SlideForward}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        setCameraModifier( CameraModifier::AllowSteepSlants );
        setCameraRotationAroundCenterX( -45_deg );
        if( getLevel().m_inputHandler->getInputState().jump )
        {
            setGoalAnimState( LaraStateId::JumpForward );
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        setMovementAngle( getLara().m_state.rotation.Y );
        commonSlideHandling( collisionInfo );
    }
};
}
}
