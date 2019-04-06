#pragma once

#include "sounds_tr1.h"
#include "floordata/floordata.h"
#include "audio/soundengine.h"
#include "core/id.h"
#include "loader/file/audio.h"

#include <map>

namespace engine
{
enum class TR1TrackId;


class Engine;


struct AudioEngine
{
    Engine& m_engine;

    std::vector<loader::file::SoundDetails> m_soundDetails;
    std::vector<int16_t> m_soundmap;
    std::vector<uint32_t> m_sampleIndices;

    explicit AudioEngine(Engine& engine,
                         const std::vector<loader::file::SoundDetails>& soundDetails,
                         const std::vector<int16_t>& soundmap,
                         const std::vector<uint32_t>& sampleIndices)
            : m_engine{engine}
            , m_soundDetails{soundDetails}
            , m_soundmap{soundmap}
            , m_sampleIndices{sampleIndices}
    {
    }

    std::map<TR1TrackId, engine::floordata::ActivationState> m_cdTrackActivationStates;
    int m_cdTrack50time = 0;
    std::weak_ptr<audio::SourceHandle> m_underwaterAmbience;
    audio::SoundEngine m_soundEngine;
    std::weak_ptr<audio::Stream> m_ambientStream;
    std::weak_ptr<audio::Stream> m_interceptStream;
    boost::optional<TR1TrackId> m_currentTrack;
    boost::optional<TR1SoundId> m_currentLaraTalk;

    std::shared_ptr<audio::SourceHandle> playSound(core::SoundId id, audio::Emitter* emitter);

    std::shared_ptr<audio::SourceHandle> playSound(const core::SoundId id, const glm::vec3& pos)
    {
        const auto handle = playSound( id, nullptr );
        handle->setPosition( pos );
        return handle;
    }

    gsl::not_null<std::shared_ptr<audio::Stream>> playStream(size_t trackId);

    void playStopCdTrack(TR1TrackId trackId, bool stop);

    void triggerNormalCdTrack(TR1TrackId trackId,
                              const floordata::ActivationState& activationRequest,
                              floordata::SequenceCondition triggerType);

    void triggerCdTrack(TR1TrackId trackId,
                        const floordata::ActivationState& activationRequest,
                        floordata::SequenceCondition triggerType);

    void stopSound(core::SoundId soundId, audio::Emitter* emitter);

    void setUnderwater(bool underwater);
};
}