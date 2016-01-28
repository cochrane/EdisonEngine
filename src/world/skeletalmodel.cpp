#include "skeletalmodel.h"

#include "core/basemesh.h"
#include "engine/engine.h"
#include "loader/level.h"
#include "resource.h"
#include "strings.h"
#include "world.h"

namespace world
{
void SkeletalModel::clear()
{
    m_meshReferences.clear();
    m_collisionMap.clear();
    m_animations.clear();
}

void SkeletalModel::updateTransparencyFlag()
{
    m_hasTransparency = std::any_of(m_meshReferences.begin(), m_meshReferences.end(),
                                   [](const MeshReference& mesh) { return !mesh.mesh_base->m_transparencyPolygons.empty(); }
    );
}

void SkeletalModel::fillSkinnedMeshMap()
{
    for(MeshReference& mesh : m_meshReferences)
    {
        if(!mesh.mesh_skin)
        {
            continue;
        }

        mesh.mesh_skin->m_matrixIndices.clear();
        for(core::Vertex& v : mesh.mesh_skin->m_vertices)
        {
            if(const core::Vertex* rv = mesh.mesh_base->findVertex(v.position))
            {
                mesh.mesh_skin->m_matrixIndices.emplace_back(0, 0);
                v.position = rv->position;
                v.normal = rv->normal;
                continue;
            }

            mesh.mesh_skin->m_matrixIndices.emplace_back(0, 1);
            glm::vec3 tv = v.position + mesh.offset;
            for(const MeshReference& prevMesh : m_meshReferences)
            {
                const core::Vertex* rv = prevMesh.mesh_base->findVertex(tv);
                if(rv == nullptr)
                    continue;

                mesh.mesh_skin->m_matrixIndices.emplace_back(1, 1);
                v.position = rv->position - mesh.offset;
                v.normal = rv->normal;
                break;
            }
        }
    }
}

void SkeletalModel::setMeshes(const std::vector<SkeletalModel::MeshReference>& src, size_t meshCount)
{
    BOOST_ASSERT(meshCount <= m_meshReferences.size() && meshCount <= src.size());
    for(size_t i = 0; i < meshCount; i++)
    {
        m_meshReferences[i].mesh_base = src[i].mesh_base;
    }
}

void SkeletalModel::setSkinnedMeshes(const std::vector<SkeletalModel::MeshReference>& src, size_t meshCount)
{
    BOOST_ASSERT(meshCount <= m_meshReferences.size() && meshCount <= src.size());
    for(size_t i = 0; i < meshCount; i++)
    {
        m_meshReferences[i].mesh_skin = src[i].mesh_base;
    }
}

/**
* Find a possible state change to \c stateid
* \param[in]      stateid  the desired target state
* \param[in,out]  animid   reference to anim id, receives found anim
* \param[in,out]  frameid  reference to frame id, receives found frame
* \return  true if state is found, false otherwise
*/
bool SkeletalModel::findStateChange(LaraState stateid, animation::AnimationId& animid_inout, size_t& frameid_inout)
{
    const animation::StateChange* stc = m_animations[animid_inout].findStateChangeByID(stateid);
    if(!stc)
        return false;

    for(const animation::AnimationDispatch& dispatch : stc->dispatches)
    {
        if(frameid_inout >= dispatch.start
           && frameid_inout <= dispatch.end)
        {
            animid_inout = dispatch.next.animation;
            frameid_inout = dispatch.next.frame;
            return true;
        }
    }

    return false;
}

void SkeletalModel::generateAnimCommands(const World& world)
{
    
    if(world.m_animCommands.empty())
    {
        return;
    }
    //Sys_DebugLog("anim_transform.txt", "MODEL[%d]", model.id);
    for(animation::Animation& anim : m_animations)
    {
        // Parse AnimCommands
        // Max. amount of AnimCommands is 255, larger numbers are considered as 0.
        // See http://evpopov.com/dl/TR4format.html#Animations for details.
        if(anim.animationCommandCount > 255)
        {
            continue;                                                           // If no anim commands or current anim has more than 255 (according to TRosettaStone).
        }

        if(anim.animationCommandCount == 0)
            continue;

        BOOST_ASSERT(anim.animationCommand < world.m_animCommands.size());
        const int16_t *pointer = &world.m_animCommands[anim.animationCommand];

        for(size_t i = 0; i < anim.animationCommandCount; i++)
        {
            const auto command = static_cast<animation::AnimCommandOpcode>(*pointer);
            ++pointer;
            switch(command)
            {
                /*
                * End-of-anim commands:
                */
                case animation::AnimCommandOpcode::SetPosition:
                    anim.finalAnimCommands.push_back({ command, pointer[0], pointer[1], pointer[2] });
                    // ConsoleInfo::instance().printf("ACmd MOVE: anim = %d, x = %d, y = %d, z = %d", static_cast<int>(anim), pointer[0], pointer[1], pointer[2]);
                    pointer += 3;
                    break;

                case animation::AnimCommandOpcode::SetVelocity:
                    anim.finalAnimCommands.push_back({ command, pointer[0], pointer[1], 0 });
                    // ConsoleInfo::instance().printf("ACmd JUMP: anim = %d, vVert = %d, vHoriz = %d", static_cast<int>(anim), pointer[0], pointer[1]);
                    pointer += 2;
                    break;

                case animation::AnimCommandOpcode::EmptyHands:
                    anim.finalAnimCommands.push_back({ command, 0, 0, 0 });
                    // ConsoleInfo::instance().printf("ACmd EMTYHANDS: anim = %d", static_cast<int>(anim));
                    break;

                case animation::AnimCommandOpcode::Kill:
                    anim.finalAnimCommands.push_back({ command, 0, 0, 0 });
                    // ConsoleInfo::instance().printf("ACmd KILL: anim = %d", static_cast<int>(anim));
                    break;

                    /*
                    * Per frame commands:
                    */
                case animation::AnimCommandOpcode::PlaySound:
                    if(pointer[0] - anim.frameStart < static_cast<int>(anim.getFrameDuration()))
                    {
                        anim.animCommands(pointer[0] - anim.frameStart).push_back({ command, pointer[1], 0, 0 });
                    }
                    // ConsoleInfo::instance().printf("ACmd PLAYSOUND: anim = %d, frame = %d of %d", static_cast<int>(anim), pointer[0], static_cast<int>(af->frames.size()));
                    pointer += 2;
                    break;

                case animation::AnimCommandOpcode::PlayEffect:
                    if(pointer[0] < static_cast<int>(anim.getFrameDuration()))
                    {
                        anim.animCommands(pointer[0] - anim.frameStart).push_back({ command, pointer[1], 0, 0 });
                    }
                    //                    ConsoleInfo::instance().printf("ACmd FLIPEFFECT: anim = %d, frame = %d of %d", static_cast<int>(anim), pointer[0], static_cast<int>(af->frames.size()));
                    pointer += 2;
                    break;
            }
        }
    }
}

void SkeletalModel::loadStateChanges(const World& world, const loader::Level& level, const loader::Moveable& moveable)
{
#ifdef LOG_ANIM_DISPATCHES
    if(animations.size() > 1)
    {
        BOOST_LOG_TRIVIAL(debug) << "MODEL[" << model_num << "], anims = " << animations.size();
    }
#endif
    for(size_t i = 0; i < m_animations.size(); i++)
    {
        animation::Animation* anim = &m_animations[i];
        anim->stateChanges.clear();

        const loader::Animation& trAnimation = level.m_animations[moveable.animation_index + i];
        int16_t animId = trAnimation.next_animation - moveable.animation_index;
        animId &= 0x7fff; // this masks out the sign bit
        BOOST_ASSERT(animId >= 0);
        if(static_cast<size_t>(animId) < m_animations.size())
        {
            anim->next_anim = &m_animations[animId];
            anim->next_frame = trAnimation.next_frame - level.m_animations[trAnimation.next_animation].frame_start;
            anim->next_frame %= anim->next_anim->getFrameDuration();
#ifdef LOG_ANIM_DISPATCHES
            BOOST_LOG_TRIVIAL(debug) << "ANIM[" << i << "], next_anim = " << anim->next_anim->id << ", next_frame = " << anim->next_frame;
#endif
        }
        else
        {
            anim->next_anim = nullptr;
            anim->next_frame = 0;
        }

        anim->stateChanges.clear();

        if(trAnimation.num_state_changes > 0 && m_animations.size() > 1)
        {
#ifdef LOG_ANIM_DISPATCHES
            BOOST_LOG_TRIVIAL(debug) << "ANIM[" << i << "], next_anim = " << (anim->next_anim ? anim->next_anim->id : -1) << ", next_frame = " << anim->next_frame;
#endif
            for(uint16_t j = 0; j < trAnimation.num_state_changes; j++)
            {
                const loader::StateChange *tr_sch = &level.m_stateChanges[j + trAnimation.state_change_offset];
                if(anim->findStateChangeByID(static_cast<LaraState>(tr_sch->state_id)) != nullptr)
                {
                    BOOST_LOG_TRIVIAL(warning) << "Multiple state changes for id " << tr_sch->state_id;
                }
                animation::StateChange* sch_p = &anim->stateChanges[static_cast<LaraState>(tr_sch->state_id)];
                sch_p->id = static_cast<LaraState>(tr_sch->state_id);
                sch_p->dispatches.clear();
                for(uint16_t l = 0; l < tr_sch->num_anim_dispatches; l++)
                {
                    const loader::AnimDispatch *tr_adisp = &level.m_animDispatches[tr_sch->anim_dispatch + l];
                    uint16_t next_anim = tr_adisp->next_animation & 0x7fff;
                    uint16_t next_anim_ind = next_anim - (moveable.animation_index & 0x7fff);
                    if(next_anim_ind >= m_animations.size())
                        continue;

                    sch_p->dispatches.emplace_back();

                    animation::AnimationDispatch* adsp = &sch_p->dispatches.back();
                    size_t next_frames_count = m_animations[next_anim - moveable.animation_index].getFrameDuration();
                    size_t next_frame = tr_adisp->next_frame - level.m_animations[next_anim].frame_start;

                    uint16_t low = tr_adisp->low - trAnimation.frame_start;
                    uint16_t high = tr_adisp->high - trAnimation.frame_start;

                    // this is not good: frame_high can be frame_end+1 (for last-frame-loop statechanges,
                    // secifically fall anims (75,77 etc), which may fail to change state),
                    // And: if theses values are > framesize, then they're probably faulty and won't be fixed by modulo anyway:
                    //                        adsp->frame_low = low  % anim->frames.size();
                    //                        adsp->frame_high = (high - 1) % anim->frames.size();
                    if(low > anim->getFrameDuration() || high > anim->getFrameDuration())
                    {
                        //Sys_Warn("State range out of bounds: anim: %d, stid: %d, low: %d, high: %d", anim->id, sch_p->id, low, high);
                        world.m_engine->m_gui.getConsole().printf("State range out of bounds: anim: %d, stid: %d, low: %d, high: %d, duration: %d, timestretch: %d", anim->id, sch_p->id, low, high, int(anim->getFrameDuration()), int(anim->getStretchFactor()));
                    }
                    adsp->start = low;
                    adsp->end = high;
                    BOOST_ASSERT(next_anim >= moveable.animation_index);
                    adsp->next.animation = next_anim - moveable.animation_index;
                    adsp->next.frame = next_frame % next_frames_count;

#ifdef LOG_ANIM_DISPATCHES
                    BOOST_LOG_TRIVIAL(debug) << "anim_disp["
                        << l
                        << "], duration = "
                        << anim->getFrameDuration()
                        << ": interval["
                        << adsp->frame_low
                        << ".."
                        << adsp->frame_high
                        << "], next_anim = "
                        << adsp->next.animation
                        << ", next_frame = "
                        << adsp->next.frame;
#endif
                }
            }
        }
    }
}

void SkeletalModel::setStaticAnimation()
{
    m_animations.resize(1);
    m_animations.front().setDuration(1, 1, 1);
    animation::SkeletonKeyFrame& keyFrame = m_animations.front().rawKeyFrame(0);

    m_animations.front().id = 0;
    m_animations.front().next_anim = nullptr;
    m_animations.front().next_frame = 0;
    m_animations.front().stateChanges.clear();

    keyFrame.boneKeyFrames.resize(m_meshReferences.size());

    keyFrame.position = { 0,0,0 };

    for(size_t k = 0; k < keyFrame.boneKeyFrames.size(); k++)
    {
        animation::BoneKeyFrame& boneKeyFrame = keyFrame.boneKeyFrames[k];

        boneKeyFrame.qrotate = util::vec4_SetTRRotations({ 0,0,0 });
        boneKeyFrame.offset = m_meshReferences[k].offset;
    }
}

void SkeletalModel::loadAnimations(const loader::Level& level, size_t moveable)
{
    m_animations.resize(getKeyframeCountForMoveable(level, moveable));
    if(m_animations.empty())
    {
        /*
        * the animation count must be >= 1
        */
        m_animations.resize(1);
    }

    /*
    *   Ok, let us calculate animations;
    *   there is no difficult:
    * - first 9 words are bounding box and frame offset coordinates.
    * - 10's word is a rotations count, must be equal to number of meshes in model.
    *   BUT! only in TR1. In TR2 - TR5 after first 9 words begins next section.
    * - in the next follows rotation's data. one word - one rotation, if rotation is one-axis (one angle).
    *   two words in 3-axis rotations (3 angles). angles are calculated with bit mask.
    */
    for(size_t i = 0; i < m_animations.size(); i++)
    {
        animation::Animation* anim = &m_animations[i];
        const loader::Animation& trAnimation = level.m_animations[level.m_moveables[moveable]->animation_index + i];
        anim->frameStart = trAnimation.frame_start;

        uint32_t frame_offset = trAnimation.frame_offset / 2;
        uint16_t l_start = 0x09;

        if(level.m_gameVersion == loader::Game::TR1 || level.m_gameVersion == loader::Game::TR1Demo || level.m_gameVersion == loader::Game::TR1UnfinishedBusiness)
        {
            l_start = 0x0A;
        }

        anim->id = i;
        BOOST_LOG_TRIVIAL(debug) << "Anim " << i << " stretch factor = " << int(trAnimation.frame_rate) << ", frame count = " << (trAnimation.frame_end - trAnimation.frame_start + 1);

        anim->speed_x = trAnimation.speed;
        anim->accel_x = trAnimation.accel;
        anim->speed_y = trAnimation.accel_lateral;
        anim->accel_y = trAnimation.speed_lateral;

        anim->animationCommand = trAnimation.anim_command;
        anim->animationCommandCount = trAnimation.num_anim_commands;
        anim->state_id = static_cast<LaraState>(trAnimation.state_id);

        //        anim->frames.resize(TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index + i));
        // FIXME: number of frames is always (frame_end - frame_start + 1)
        // this matters for transitional anims, which run through their frame length with the same anim frame.
        // This should ideally be solved without filling identical frames,
        // but due to the amount of currFrame-indexing, waste dummy frames for now:
        // (I haven't seen this for framerate==1 animation, but it would be possible,
        //  also, resizing now saves re-allocations in interpolateFrames later)

        const size_t keyFrameCount = getKeyframeCountForMoveable(level, level.m_moveables[moveable]->animation_index + i);
        anim->setDuration(trAnimation.frame_end - trAnimation.frame_start + 1, keyFrameCount, trAnimation.frame_rate);

        //Sys_DebugLog(LOG_FILENAME, "Anim[%d], %d", tr_moveable->animation_index, TR_GetNumFramesForAnimation(tr, tr_moveable->animation_index));

        if(anim->getFrameDuration() == 0)
        {
            /*
            * number of animations must be >= 1, because frame contains base model offset
            */
            anim->setDuration(1, 1, anim->getStretchFactor());
        }

        /*
        * let us begin to load animations
        */
        for(size_t j = 0; j < anim->getKeyFrameCount(); ++j, frame_offset += trAnimation.frame_size)
        {
            animation::SkeletonKeyFrame* keyFrame = &anim->rawKeyFrame(j);
            // !Need bonetags in empty frames:
            keyFrame->boneKeyFrames.resize(m_meshReferences.size());

            if(j >= keyFrameCount)
            {
                BOOST_LOG_TRIVIAL(warning) << "j=" << j << ", keyFrameCount=" << keyFrameCount << ", anim->getKeyFrameCount()=" << anim->getKeyFrameCount();
                continue;
            }

            keyFrame->position = { 0,0,0 };
            keyFrame->load(level, frame_offset);

            if(frame_offset >= level.m_frameData.size())
            {
                for(size_t k = 0; k < keyFrame->boneKeyFrames.size(); k++)
                {
                    animation::BoneKeyFrame* boneKeyFrame = &keyFrame->boneKeyFrames[k];
                    boneKeyFrame->qrotate = util::vec4_SetTRRotations({ 0,0,0 });
                    boneKeyFrame->offset = m_meshReferences[k].offset;
                }
                continue;
            }

            uint16_t l = l_start;
            uint16_t temp1, temp2;
            float ang;

            for(size_t k = 0; k < keyFrame->boneKeyFrames.size(); k++)
            {
                animation::BoneKeyFrame* bone_tag = &keyFrame->boneKeyFrames[k];
                bone_tag->qrotate = util::vec4_SetTRRotations({ 0,0,0 });
                bone_tag->offset = m_meshReferences[k].offset;

                if(loader::gameToEngine(level.m_gameVersion) == loader::Engine::TR1)
                {
                    temp2 = level.m_frameData[frame_offset + l];
                    l++;
                    temp1 = level.m_frameData[frame_offset + l];
                    l++;
                    glm::vec3 rot;
                    rot[0] = static_cast<float>((temp1 & 0x3ff0) >> 4);
                    rot[2] = -static_cast<float>(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                    rot[1] = static_cast<float>(temp2 & 0x03ff);
                    rot *= 360.0 / 1024.0;
                    bone_tag->qrotate = util::vec4_SetTRRotations(rot);
                }
                else
                {
                    temp1 = level.m_frameData[frame_offset + l];
                    l++;
                    if(level.m_gameVersion >= loader::Game::TR4)
                    {
                        ang = static_cast<float>(temp1 & 0x0fff);
                        ang *= 360.0 / 4096.0;
                    }
                    else
                    {
                        ang = static_cast<float>(temp1 & 0x03ff);
                        ang *= 360.0 / 1024.0;
                    }

                    switch(temp1 & 0xc000)
                    {
                        case 0x4000:    // x only
                            bone_tag->qrotate = util::vec4_SetTRRotations({ ang,0,0 });
                            break;

                        case 0x8000:    // y only
                            bone_tag->qrotate = util::vec4_SetTRRotations({ 0,0,-ang });
                            break;

                        case 0xc000:    // z only
                            bone_tag->qrotate = util::vec4_SetTRRotations({ 0,ang,0 });
                            break;

                        default:
                        {        // all three
                            temp2 = level.m_frameData[frame_offset + l];
                            glm::vec3 rot;
                            rot[0] = static_cast<float>((temp1 & 0x3ff0) >> 4);
                            rot[2] = -static_cast<float>(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                            rot[1] = static_cast<float>(temp2 & 0x03ff);
                            rot *= 360.0 / 1024.0;
                            bone_tag->qrotate = util::vec4_SetTRRotations(rot);
                            l++;
                            break;
                        }
                    };
                }
            }
        }
    }
}

size_t SkeletalModel::getKeyframeCountForMoveable(const loader::Level& level, size_t moveable)
{
    const std::unique_ptr<loader::Moveable>& curr_moveable = level.m_moveables[moveable];

    if(curr_moveable->animation_index == 0xFFFF)
    {
        return 0;
    }

    if(moveable == level.m_moveables.size() - 1)
    {
        if(level.m_animations.size() < curr_moveable->animation_index)
        {
            return 1;
        }

        return level.m_animations.size() - curr_moveable->animation_index;
    }

    const loader::Moveable* next_moveable = level.m_moveables[moveable + 1].get();
    if(next_moveable->animation_index == 0xFFFF)
    {
        if(moveable + 2 < level.m_moveables.size())                              // I hope there is no two neighboard movables with animation_index'es == 0xFFFF
        {
            next_moveable = level.m_moveables[moveable + 2].get();
        }
        else
        {
            return 1;
        }
    }

    return std::min(static_cast<size_t>(next_moveable->animation_index), level.m_animations.size()) - curr_moveable->animation_index;
}

void SkeletalModel::patchLaraSkin(World& world, loader::Engine engineVersion)
{
    BOOST_ASSERT(m_id == 0);

    switch(engineVersion)
    {
        case loader::Engine::TR1:
            if(world.m_engine->gameflow.getLevelID() == 0)
            {
                if(std::shared_ptr<SkeletalModel> skinModel = world.getModelByID(TR_ITEM_LARA_SKIN_ALTERNATE_TR1))
                {
                    // In TR1, Lara has unified head mesh for all her alternate skins.
                    // Hence, we copy all meshes except head, to prevent Potato Raider bug.
                    setMeshes(skinModel->m_meshReferences, m_meshReferences.size() - 1);
                }
            }
            break;

        case loader::Engine::TR2:
            break;

        case loader::Engine::TR3:
            if(std::shared_ptr<SkeletalModel> skinModel = world.getModelByID(TR_ITEM_LARA_SKIN_TR3))
            {
                setMeshes(skinModel->m_meshReferences, m_meshReferences.size());
                auto tmp = world.getModelByID(11);                   // moto / quadro cycle animations
                if(tmp)
                {
                    tmp->setMeshes(skinModel->m_meshReferences, m_meshReferences.size());
                }
            }
            break;

        case loader::Engine::TR4:
        case loader::Engine::TR5:
            // base skeleton meshes
            if(std::shared_ptr<SkeletalModel> skinModel = world.getModelByID(TR_ITEM_LARA_SKIN_TR45))
            {
                setMeshes(skinModel->m_meshReferences, m_meshReferences.size());
            }
            // skin skeleton meshes
            if(std::shared_ptr<SkeletalModel> skinModel = world.getModelByID(TR_ITEM_LARA_SKIN_JOINTS_TR45))
            {
                setSkinnedMeshes(skinModel->m_meshReferences, m_meshReferences.size());
            }
            fillSkinnedMeshMap();
            break;

        case loader::Engine::Unknown:
            break;
    };
}

void SkeletalModel::lua_SetModelMeshReplaceFlag(engine::Engine& engine, ModelId id, size_t bone, int flag)
{
    auto sm = engine.m_world.getModelByID(id);
    if(sm != nullptr)
    {
        if(bone < sm->getMeshReferenceCount())
        {
            sm->m_meshReferences[bone].replace_mesh = flag;
        }
        else
        {
            engine.m_gui.getConsole().warning(SYSWARN_WRONG_BONE_NUMBER, bone);
        }
    }
    else
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_MODEL_ID, id);
    }
}

void SkeletalModel::lua_SetModelAnimReplaceFlag(engine::Engine& engine, ModelId id, size_t bone, bool flag)
{
    auto sm = engine.m_world.getModelByID(id);
    if(sm != nullptr)
    {
        if(bone < sm->getMeshReferenceCount())
        {
            sm->m_meshReferences[bone].replace_anim = flag;
        }
        else
        {
            engine.m_gui.getConsole().warning(SYSWARN_WRONG_BONE_NUMBER, bone);
        }
    }
    else
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_MODEL_ID, id);
    }
}

void SkeletalModel::lua_CopyMeshFromModelToModel(engine::Engine& engine, ModelId id1, ModelId id2, size_t bone1, size_t bone2)
{
    auto sm1 = engine.m_world.getModelByID(id1);
    if(sm1 == nullptr)
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_MODEL_ID, id1);
        return;
    }

    auto sm2 = engine.m_world.getModelByID(id2);
    if(sm2 == nullptr)
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_MODEL_ID, id2);
        return;
    }

    if(bone1 < sm1->getMeshReferenceCount() && bone2 < sm2->getMeshReferenceCount())
    {
        sm1->m_meshReferences[bone1].mesh_base = sm2->m_meshReferences[bone2].mesh_base;
    }
    else
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_BONE_NUMBER, bone1);
    }
}

void SkeletalModel::lua_SetModelBodyPartFlag(engine::Engine& engine, ModelId id, int bone_id, int body_part_flag)
{
    auto model = engine.m_world.getModelByID(id);

    if(model == nullptr)
    {
        engine.m_gui.getConsole().warning(SYSWARN_NO_SKELETAL_MODEL, id);
        return;
    }

    if(bone_id < 0 || static_cast<size_t>(bone_id) >= model->getMeshReferenceCount())
    {
        engine.m_gui.getConsole().warning(SYSWARN_WRONG_OPTION_INDEX, bone_id);
        return;
    }

    model->m_meshReferences[bone_id].body_part = body_part_flag;
}
} // namespace world
