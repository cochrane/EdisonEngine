#pragma once

#include "modelobject.h"

namespace engine::objects
{
class BridgeFlat final : public ModelObject
{
public:
  BridgeFlat(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  BridgeFlat(const gsl::not_null<World*>& world,
             const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, false, animatedModel}
  {
  }

  void patchFloor(const core::TRVec& pos, core::Length& y) override
  {
    if(pos.Y <= m_state.position.position.Y)
      y = m_state.position.position.Y;
  }

  void patchCeiling(const core::TRVec& pos, core::Length& y) override
  {
    if(pos.Y <= m_state.position.position.Y)
      return;

    y = m_state.position.position.Y + core::QuarterSectorSize;
  }
};
} // namespace engine::objects
