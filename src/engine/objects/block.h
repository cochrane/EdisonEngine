#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Block final : public ModelObject
{
public:
  Block(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  Block(const gsl::not_null<World*>& world,
        const gsl::not_null<const loader::file::Room*>& room,
        const loader::file::Item& item,
        const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
    if(m_state.triggerState != TriggerState::Invisible)
    {
      loader::file::Room::patchHeightsForBlock(*this, -core::SectorSize);
    }
  }

  void collide(CollisionInfo& collisionInfo) override;

  void update() override;

private:
  bool isOnFloor(const core::Length& height) const;

  bool canPushBlock(const core::Length& height, core::Axis axis) const;

  bool canPullBlock(const core::Length& height, core::Axis axis) const;
};
} // namespace engine::objects
