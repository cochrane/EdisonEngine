#include "lavaparticleemitter.h"

#include "engine/particle.h"
#include "engine/world.h"
#include "laraobject.h"

namespace engine::objects
{
void LavaParticleEmitter::update()
{
  auto particle = std::make_shared<LavaParticle>(m_state.position, getWorld());
  setParent(particle, m_state.position.room->node);
  getWorld().getObjectManager().registerParticle(particle);

  playSoundEffect(TR1SoundEffect::ChoppyWater);
}

void LavaParticleEmitter::serialize(const serialization::Serializer& ser)
{
  ModelObject::serialize(ser);
  getSkeleton()->setRenderable(nullptr);
  getSkeleton()->removeAllChildren();
  getSkeleton()->clearParts();
}
} // namespace engine::objects
