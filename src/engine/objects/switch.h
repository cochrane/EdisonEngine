#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Switch : public ModelObject
{
public:
  Switch(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  Switch(const gsl::not_null<World*>& world,
         const gsl::not_null<const loader::file::Room*>& room,
         const loader::file::Item& item,
         const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void collide(CollisionInfo& collisionInfo) override;

  void update() final
  {
    m_state.activationState.fullyActivate();
    if(!m_state.updateActivationTimeout())
    {
      // reset switch if a timeout is active
      m_state.goal_anim_state = 1_as;
      m_state.timer = 0_frame;
    }

    ModelObject::update();
  }
};
} // namespace engine::objects
