#pragma once

#include "level.h"

namespace level
{
class TR1Level : public Level
{
public:
    TR1Level(Game gameVersion, loader::io::SDLReader&& reader)
        : Level(gameVersion, std::move(reader))
    {
    }

    void load(irr::video::IVideoDriver* drv) override;
};
} // namespace loader