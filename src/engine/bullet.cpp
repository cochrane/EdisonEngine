#include "bullet.h"

#include "engine.h"
#include "game.h"
#include "render/render.h"
#include "script/script.h"
#include "util/vmath.h"
#include "world/character.h"
#include "world/room.h"

#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <boost/range/adaptors.hpp>

namespace engine
{
namespace
{
/**
 * overlapping room collision filter
 */
void roomNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
{
    world::Object* c0 = static_cast<world::Object*>(static_cast<btCollisionObject*>(collisionPair.m_pProxy0->m_clientObject)->getUserPointer());
    world::Room* r0 = c0 ? c0->getRoom() : nullptr;
    world::Object* c1 = static_cast<world::Object*>(static_cast<btCollisionObject*>(collisionPair.m_pProxy1->m_clientObject)->getUserPointer());
    world::Room* r1 = c1 ? c1->getRoom() : nullptr;

    if(c1 && c1 == c0)
    {
        if(static_cast<btCollisionObject*>(collisionPair.m_pProxy0->m_clientObject)->isStaticOrKinematicObject() ||
           static_cast<btCollisionObject*>(collisionPair.m_pProxy1->m_clientObject)->isStaticOrKinematicObject())
        {
            return;                                                             // No self interaction
        }
        dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
        return;
    }

    if(!r0 && !r1)
    {
        dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);// Both are out of rooms
        return;
    }

    if(r0 && r1 && r0->isInNearRoomsList(*r1))
    {
        dispatcher.defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
    }
}

void storeEntityLerpTransforms()
{
    if(engine_world.character && !(engine_world.character->m_typeFlags & ENTITY_TYPE_DYNAMIC))
    {
        // set bones to next interval step, this keeps the ponytail (bullet's dynamic interpolation) in sync with actor interpolation:
        engine_world.character->m_skeleton.updatePose();
        engine_world.character->updateRigidBody(false);
        engine_world.character->ghostUpdate();
    }

    for(const std::shared_ptr<world::Entity>& entity : engine_world.entity_tree | boost::adaptors::map_values)
    {
        if(!entity->m_enabled)
            continue;

        if((entity->m_typeFlags & ENTITY_TYPE_DYNAMIC) != 0)
            continue;

        entity->m_skeleton.updatePose();
        entity->updateRigidBody(false);
        entity->ghostUpdate();
    }
}

/**
 * Pre-physics step callback
 */
void internalPreTickCallback(btDynamicsWorld* /*world*/, float timeStep)
{
    util::Duration engine_frame_time_backup = engine_frame_time;
    engine_frame_time = util::fromSeconds(timeStep);

    engine_lua.doTasks(engine_frame_time_backup);
    Game_UpdateAI();
    engine_world.audioEngine.updateAudio();

    if(engine_world.character)
    {
        engine_world.character->frame(util::fromSeconds(timeStep));
    }
    for(const std::shared_ptr<world::Entity>& entity : engine_world.entity_tree | boost::adaptors::map_values)
    {
        entity->frame(util::fromSeconds(timeStep));
    }

    storeEntityLerpTransforms();
    engine_frame_time = engine_frame_time_backup;
}

/**
 * Post-physics step callback
 */
void internalTickCallback(btDynamicsWorld *world, float /*timeStep*/)
{
    // Update all physics object's transform/room:
    for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        BOOST_ASSERT(i >= 0 && i < BulletEngine::instance->dynamicsWorld->getCollisionObjectArray().size());
        btCollisionObject* obj = BulletEngine::instance->dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if(body && !body->isStaticObject() && body->getMotionState())
        {
            btTransform trans;
            body->getMotionState()->getWorldTransform(trans);
            world::Object* object = static_cast<world::Object*>(body->getUserPointer());
            if(dynamic_cast<world::BulletObject*>(object))
            {
                object->setRoom( Room_FindPosCogerrence(util::convert(trans.getOrigin()), object->getRoom()) );
            }
        }
    }
}
} // anonymous namespace

// Bullet Physics initialization.
std::unique_ptr<BulletEngine> BulletEngine::instance = nullptr;

BulletEngine::BulletEngine()
{
    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    collisionConfiguration.reset( new btDefaultCollisionConfiguration() );

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    dispatcher.reset( new btCollisionDispatcher(collisionConfiguration.get()) );
    dispatcher->setNearCallback(roomNearCallback);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    overlappingPairCache.reset( new btDbvtBroadphase() );
    ghostPairCallback.reset( new btGhostPairCallback() );
    overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback.get());

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    solver.reset( new btSequentialImpulseConstraintSolver );

    dynamicsWorld.reset( new btDiscreteDynamicsWorld(dispatcher.get(), overlappingPairCache.get(), solver.get(), collisionConfiguration.get()) );
    dynamicsWorld->setInternalTickCallback(internalTickCallback);
    dynamicsWorld->setInternalTickCallback(internalPreTickCallback, nullptr, true);
    dynamicsWorld->setGravity(btVector3(0, 0, -4500.0));

    render::debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints);
    dynamicsWorld->setDebugDrawer(&render::debugDrawer);
    //bt_engine_dynamicsWorld->getPairCache()->setInternalGhostPairCallback(bt_engine_filterCallback);
}

}
