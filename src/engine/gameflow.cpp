#include "gameflow.h"

#include <cstdio>
#include <cstdlib>

#include "engine/engine.h"
#include "gui/gui.h"
#include "script/script.h"
#include "world/world.h"

namespace engine
{

Gameflow Gameflow_Manager;

void Gameflow::init()
{
    for(GameflowAction& action : m_actions)
        action.opcode = Opcode::Sentinel;
}

void Gameflow::execute()
{
    if(!m_nextAction)
    {
        return;
    }

    bool completed = true;

    for(GameflowAction& action : m_actions)
    {
        completed = false;

        switch(action.opcode)
        {
            case Opcode::LevelComplete:
                // Switch level only when fade is complete AND all streams / sounds are unloaded!
                if(gui::getFaderStatus(gui::FaderType::LoadScreen) == gui::FaderStatus::Complete && !audio::isTrackPlaying())
                {
                    lua::tie(m_currentLevelPath, m_currentLevelName, m_currentLevelID) = engine_lua["getNextLevel"](int(m_currentGameID), int(m_currentLevelID), int(action.operand));
                    engine::loadMap(m_currentLevelPath);
                    action.opcode = Opcode::Sentinel;
                }
                else
                {
                    ///@FIXME Gameflow has NOTHING to do with faders! this should all be done elsewhere!
                    // If fadeout is in the process, we block level loading until it is complete.
                    // It is achieved by not resetting action marker and exiting the function instead.
                    continue;
                }   // end if(Gui_FadeCheck(FADER_LOADSCREEN))
                break;

            default:
                action.opcode = Opcode::Sentinel;
                break;
        }   // end switch(gameflow_manager.Operand)
    }

    if(completed)
        m_nextAction = false;    // Reset action marker!
}

/*
    Send opcode and operand to gameflow manager.
    Note: It will be added at the end of actions list.
*/

///@CRITICAL - The order MUST BE MAINTAINED.
bool Gameflow::send(Opcode opcode, int operand)
{
    for(GameflowAction& action : m_actions)///@FIXME But what if [ -1, 2, 3 -1]? We're essentially favouring the first -1 which is WRONG.
    {
        if(action.opcode == Opcode::Sentinel)
        {
            action.opcode = opcode;
            action.operand = operand;
            m_nextAction = true;///@FIXME No, we shouldn't need to modify this here.
            return true;
        }
    }
    return false;
}

} // namespace engine
