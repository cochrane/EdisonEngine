#pragma once

#include "modelobject.h"

namespace engine::objects
{
class ThorHammerBlock final : public ModelObject
{
public:
  ThorHammerBlock(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  ThorHammerBlock(const gsl::not_null<World*>& world,
                  const gsl::not_null<const loader::file::Room*>& room,
                  const loader::file::Item& item,
                  const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void collide(CollisionInfo& info) override;
};

class ThorHammerHandle final : public ModelObject
{
public:
  ThorHammerHandle(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position);

  ThorHammerHandle(const gsl::not_null<World*>& world,
                   const gsl::not_null<const loader::file::Room*>& room,
                   const loader::file::Item& item,
                   const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

  void serialize(const serialization::Serializer& ser) override;

private:
  const std::shared_ptr<ThorHammerBlock> m_block;
};
} // namespace engine::objects
