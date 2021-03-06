#include "cutsceneactors.h"

#include "engine/world.h"

namespace engine::objects
{
CutsceneActor::CutsceneActor(const gsl::not_null<World*>& world,
                             const gsl::not_null<const loader::file::Room*>& room,
                             const loader::file::Item& item,
                             const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
    : ModelObject{world, room, item, true, animatedModel}
{
  activate();
  m_state.rotation.Y = 0_deg;
}

void CutsceneActor::update()
{
  m_state.rotation.Y = getWorld().getCameraController().getEyeRotation().Y;
  m_state.position.position = getWorld().getCameraController().getTRPosition().position;
  ModelObject::update();
}

void CutsceneActor4::update()
{
  ModelObject::update(); // NOLINT(bugprone-parent-virtual-call)
}
} // namespace engine::objects
