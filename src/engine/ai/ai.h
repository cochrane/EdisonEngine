#pragma once

#include <level/level.h>
#include "engine/items/itemnode.h"

namespace engine
{
namespace items
{
class AIAgent;
}

namespace ai
{
enum class Mood
{
    Bored,
    Attack,
    Escape,
    Stalk
};

inline std::ostream& operator<<(std::ostream& str, const Mood mood)
{
    switch( mood )
    {
        case Mood::Bored:
            return str << "Bored";
        case Mood::Attack:
            return str << "Attack";
        case Mood::Escape:
            return str << "Escape";
        case Mood::Stalk:
            return str << "Stalk";
        default:
            BOOST_THROW_EXCEPTION( std::runtime_error( "Invalid mood" ) );
    }
}


struct SearchNode
{
    /**
     * @brief The next box on the path.
     */
    const loader::Box* exit_box = nullptr;
    uint16_t search_number = 0;
    const loader::Box* next_expansion = nullptr;

    void markBlocked()
    {
        search_number |= 0x8000u;
    }

    uint16_t getSearchVersion() const
    {
        return search_number & ~uint16_t( 0x8000u );
    }

    bool isBlocked() const
    {
        return (search_number & 0x8000u) != 0;
    }
};


struct LotInfo
{
    std::unordered_map<const loader::Box*, SearchNode> nodes;
    std::vector<const loader::Box*> boxes;
    const loader::Box* head = nullptr;
    const loader::Box* tail = nullptr;
    uint16_t m_searchVersion = 0;
    //! @brief Disallows entering certain boxes, marked in the @c loader::Box::overlap_index member.
    uint16_t block_mask = 0x4000;
    //! @brief Movement limits
    //! @{
    int16_t step = loader::QuarterSectorSize;
    int16_t drop = -loader::QuarterSectorSize;
    int16_t fly = 0;
    //! @}
    //! @brief The target box we need to reach
    const loader::Box* target_box = nullptr;
    const loader::Box* required_box = nullptr;
    core::TRCoordinates target;

    explicit LotInfo(const level::Level& lvl)
    {
        for( const auto& box : lvl.m_boxes )
            nodes.insert( std::make_pair( &box, SearchNode{} ) );
    }

    static gsl::span<const uint16_t> getOverlaps(const level::Level& lvl, uint16_t idx);

    void setRandomSearchTarget(const loader::Box* box)
    {
        Expects( box != nullptr );
        required_box = box;
        const auto zSize = box->zmax - box->zmin - loader::SectorSize;
        target.Z = zSize * (std::rand() & 0x7fff) / 0x8000 + box->zmin + loader::SectorSize / 2;
        const auto xSize = box->xmax - box->xmin - loader::SectorSize;
        target.X = xSize * (std::rand() & 0x7fff) / 0x8000 + box->xmin + loader::SectorSize / 2;
        if( fly != 0 )
        {
            target.Y = box->floor - 384;
        }
        else
        {
            target.Y = box->floor;
        }
    }

    bool calculateTarget(const level::Level& lvl, core::TRCoordinates& target, const engine::items::ItemState& item);

    /**
     * @brief Incrementally calculate all pathes to a specific box.
     * @param lvl The level for acquiring additional needed data.
     * @param maxDepth Maximum number of nodes of the search tree to expand at a time.
     *
     * @details
     * The algorithm performs a greedy breadth-first search, searching for all paths that lead to
     * #required_box.  While searching, @arg maxDepth limits the number of nodes expanded, so it may take multiple
     * calls to actually calculate the full path.  Until a full path is found, the nodes partially retain the old
     * pathes from a previous search.
     */
    void updatePath(const level::Level& lvl, const uint8_t maxDepth)
    {
        if (required_box != nullptr && required_box != target_box)
        {
            target_box = required_box;

            const auto targetNode = &nodes[target_box];
            if (targetNode->next_expansion == nullptr && tail != target_box)
            {
                targetNode->next_expansion = head;

                if (head == nullptr)
                    tail = target_box;

                head = target_box;
            }

            targetNode->search_number = ++m_searchVersion;
            targetNode->exit_box = nullptr;
        }

        Expects(target_box != nullptr);
        searchPath(lvl, maxDepth);
    }

    void searchPath(const level::Level& lvl, const uint8_t maxDepth)
    {
        if (head == nullptr)
        {
            tail = nullptr;
            return;
        }

        const auto zoneRef = loader::Box::getZoneRef(lvl.roomsAreSwapped, fly, step);
        const auto searchZone = head->*zoneRef;

        for (uint8_t i = 0; i < maxDepth; ++i)
        {
            if (head == nullptr)
            {
                tail = nullptr;
                return;
            }

            const auto parentNode = &nodes[head];

            for (auto childBoxIdx : getOverlaps(lvl, head->overlap_index))
            {
                childBoxIdx &= 0x7FFFu;
                const auto* childBox = &lvl.m_boxes[childBoxIdx];

                if (searchZone != childBox->*zoneRef)
                    continue;

                const auto boxHeightDiff = childBox->floor - head->floor;
                if (boxHeightDiff > step || boxHeightDiff < drop)
                    continue; // can't reach from this box, but still maybe from another one

                auto childNode = &nodes[childBox];

                if (parentNode->getSearchVersion() < childNode->getSearchVersion())
                    continue; // not yet checked if we can reach this box

                if (parentNode->isBlocked())
                {
                    if (parentNode->getSearchVersion() == childNode->getSearchVersion())
                        continue; // already visited; we don't care if the child is blocked or not

                    // mark as visited, will also mark as blocked
                    childNode->search_number = parentNode->search_number;
                }
                else
                {
                    if (parentNode->getSearchVersion() == childNode->getSearchVersion() && !childNode->isBlocked())
                        continue; // already visited and reachable

                    // mark as visited, and check if reachable
                    childNode->search_number = parentNode->search_number;
                    if ((childBox->overlap_index & block_mask) != 0)
                        childNode->markBlocked(); // can't reach this box
                    else
                        childNode->exit_box = head; // success! connect both boxes
                }

                if (childNode->next_expansion == nullptr && childBox != tail)
                    tail = nodes[tail].next_expansion = childBox; // enqueue for expansion
            }

            head = std::exchange(parentNode->next_expansion, nullptr);
        }
    }
};


struct AiInfo
{
    loader::ZoneId zone_number;
    loader::ZoneId enemy_zone;
    int32_t distance;
    bool ahead;
    bool bite;
    core::Angle angle;
    core::Angle enemy_facing;

    AiInfo(const level::Level& lvl, engine::items::ItemState& item);
};


struct CreatureInfo
{
    core::Angle head_rotation = 0_deg;
    core::Angle neck_rotation = 0_deg;

    core::Angle maximum_turn = 1_deg;
    uint16_t flags = 0;

    engine::items::ItemState* item;
    Mood mood = Mood::Bored;
    LotInfo lot;
    core::TRCoordinates target;

    CreatureInfo(const level::Level& lvl, engine::items::ItemState* item)
            : item{item}
            , lot{lvl}
    {
        Expects( item != nullptr );

        switch( item->object_number )
        {
            case 7:
            case 12:
            case 13:
            case 14:
                lot.drop = -loader::SectorSize;
                break;

            case 9:
            case 11:
            case 26:
                lot.step = 20 * loader::SectorSize;
                lot.drop = -20 * loader::SectorSize;
                lot.fly = 16;
                break;

            case 15:
                lot.step = loader::SectorSize / 2;
                lot.drop = -loader::SectorSize;
                break;

            case 18:
            case 20:
            case 23:
                lot.block_mask = 0x8000;
                break;
        }
    }

    void rotateHead(const core::Angle& angle)
    {
        const auto delta = util::clamp( angle - head_rotation, -5_deg, +5_deg );
        head_rotation = util::clamp( delta + head_rotation, -90_deg, +90_deg );
    }

    static sol::usertype<CreatureInfo> userType()
    {
        return sol::usertype<CreatureInfo>(
                sol::meta_function::construct, sol::no_constructor,
                "head_rotation", &CreatureInfo::head_rotation,
                "neck_rotation", &CreatureInfo::neck_rotation,
                "maximum_turn", &CreatureInfo::maximum_turn,
                "flags", &CreatureInfo::flags,
                "item", sol::readonly( &CreatureInfo::item ),
                "mood", &CreatureInfo::mood,
                "target", &CreatureInfo::target
        );
    }
};


void updateMood(const level::Level& lvl, const engine::items::ItemState& item, const AiInfo& aiInfo, bool violent);
}
}
