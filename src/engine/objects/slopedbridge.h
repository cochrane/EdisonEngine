#pragma once

#include "modelobject.h"

namespace engine::objects
{
class SlopedBridge : public ModelObject
{
private:
  int m_flatness;

public:
  SlopedBridge(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
      , m_flatness{0}
  {
  }

  SlopedBridge(const gsl::not_null<World*>& world,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel,
               const int flatness)
      : ModelObject{world, room, item, false, animatedModel}
      , m_flatness{flatness}
  {
  }

  void patchFloor(const core::TRVec& pos, core::Length& y) final
  {
    const auto tmp = m_state.position.position.Y + getBridgeSlopeHeight(pos) / m_flatness;
    if(pos.Y <= tmp)
      y = tmp;
  }

  void patchCeiling(const core::TRVec& pos, core::Length& y) final
  {
    const auto tmp = m_state.position.position.Y + getBridgeSlopeHeight(pos) / m_flatness;
    if(pos.Y <= tmp)
      return;

    y = tmp + core::QuarterSectorSize;
  }

  void serialize(const serialization::Serializer& ser) override
  {
    ModelObject::serialize(ser);
    ser(S_NV("flatness", m_flatness));
  }

private:
  core::Length getBridgeSlopeHeight(const core::TRVec& pos) const
  {
    auto axis = axisFromAngle(m_state.rotation.Y, 1_deg);
    Expects(axis.has_value());

    switch(*axis)
    {
    case core::Axis::PosZ: return core::SectorSize - 1_len - (pos.X % core::SectorSize);
    case core::Axis::PosX: return (pos.Z % core::SectorSize);
    case core::Axis::NegZ: return (pos.X % core::SectorSize);
    case core::Axis::NegX: return core::SectorSize - 1_len - (pos.Z % core::SectorSize);
    default: return 0_len;
    }
  }
};

class BridgeSlope1 final : public SlopedBridge
{
public:
  BridgeSlope1(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : SlopedBridge{world, position}
  {
  }

  BridgeSlope1(const gsl::not_null<World*>& world,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : SlopedBridge{world, room, item, animatedModel, 4}
  {
  }
};

class BridgeSlope2 final : public SlopedBridge
{
public:
  BridgeSlope2(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : SlopedBridge{world, position}
  {
  }

  BridgeSlope2(const gsl::not_null<World*>& world,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : SlopedBridge{world, room, item, animatedModel, 2}
  {
  }
};
} // namespace engine::objects
