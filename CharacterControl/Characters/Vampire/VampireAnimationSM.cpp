#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "VampireAnimationSM.h"
#include "Vampire.h"

using namespace PE::Components;
using namespace PE::Events;

namespace CharacterControl {

namespace Events {
	PE_IMPLEMENT_CLASS1(VampireAnimSM_Event_STOP, Event);

	PE_IMPLEMENT_CLASS1(VampireAnimSM_Event_WALK, Event);
}

namespace Components {

PE_IMPLEMENT_CLASS1(VampireAnimationSM, Component);

VampireAnimationSM::VampireAnimationSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself) : DefaultAnimationSM(context, arena, hMyself)
{
	m_curId = NONE;
}

void VampireAnimationSM::addDefaultComponents()
{
	DefaultAnimationSM::addDefaultComponents();
	
	PE_REGISTER_EVENT_HANDLER(Events::VampireAnimSM_Event_WALK, VampireAnimationSM::do_VampireAnimSM_Event_WALK);
	PE_REGISTER_EVENT_HANDLER(Events::VampireAnimSM_Event_STOP, VampireAnimationSM::do_VampireAnimSM_Event_STOP);
}

void VampireAnimationSM::do_VampireAnimSM_Event_STOP(PE::Events::Event* pEvt)
{

	if (m_curId != VampireAnimationSM::STAND)
	{
		m_curId = VampireAnimationSM::STAND;

		setAnimation(0, VampireAnimationSM::STAND,
			0, 0, 1, 1,
			PE::LOOPING);
	}
}

void VampireAnimationSM::do_VampireAnimSM_Event_WALK(PE::Events::Event *pEvt)
{
	if(m_curId != VampireAnimationSM::WALK)
	{
		m_curId = VampireAnimationSM::WALK;
		setAnimation(0, VampireAnimationSM::WALK,
			0, 0, 1, 1,
			PE::LOOPING);
	}
}

};//namespace Components
};//namespace CharacterControl