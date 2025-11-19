#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "PlayerAnimationSM.h"
#include "Player.h"

using namespace PE::Components;
using namespace PE::Events;

namespace CharacterControl{

namespace Events{
PE_IMPLEMENT_CLASS1(PlayerAnimSM_Event_STOP, Event);

PE_IMPLEMENT_CLASS1(PlayerAnimSM_Event_WALK, Event);

}
namespace Components{

PE_IMPLEMENT_CLASS1(PlayerAnimationSM, DefaultAnimationSM);

PlayerAnimationSM::PlayerAnimationSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself) : DefaultAnimationSM(context, arena, hMyself)
{
	m_curId = NONE;
}

void PlayerAnimationSM::addDefaultComponents()
{
	DefaultAnimationSM::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(Events::PlayerAnimSM_Event_STOP, PlayerAnimationSM::do_PlayerAnimSM_Event_STOP);
	PE_REGISTER_EVENT_HANDLER(Events::PlayerAnimSM_Event_WALK, PlayerAnimationSM::do_PlayerAnimSM_Event_WALK);
}

void PlayerAnimationSM::do_PlayerAnimSM_Event_STOP(PE::Events::Event *pEvt)
{
	
	if (m_curId != PlayerAnimationSM::STAND)
	{
		m_curId = PlayerAnimationSM::STAND;
		
		setAnimation(0, PlayerAnimationSM::STAND,
		0, 0, 1, 1,
		PE::LOOPING);
	}
}

void PlayerAnimationSM::do_PlayerAnimSM_Event_WALK(PE::Events::Event *pEvt)
{
	if (m_curId != PlayerAnimationSM::WALK)
	{
		m_curId = PlayerAnimationSM::WALK;
		setAnimation(0, PlayerAnimationSM::WALK,
			0, 0, 1, 1,
			PE::LOOPING);
	}
}


}}




