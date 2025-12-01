#ifndef _CHARACTER_VAMPIRE_BEHAVIOR_SM_H_
#define _CHARACTER_VAMPIRE_BEHAVIOR_SM_H_

#include "PrimeEngine/Events/Component.h"

#include "CharacterControl/Events/Events.h"

namespace CharacterControl {

namespace Components {

struct VampireBehaviorSM : public PE::Components::Component
{
	PE_DECLARE_CLASS(VampireBehaviorSM);

	enum States
	{
		IDLE,
		WAITING_FOR_WAYPOINT,
		PATROLLING_WAYPOINTS,
	};

	VampireBehaviorSM(PE::GameContext& context, PE::MemoryArena arena, PE::Handle hMyself, PE::Handle hMovementSM);

	void start();

	//Compoent API and Event handlers
	virtual void addDefaultComponents();

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_VampireMovementSM_Event_TARGET_REACHED);
	virtual void do_VampireMovementSM_Event_TARGET_REACHED(PE::Events::Event* pEvt);

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
	virtual void do_UPDATE(PE::Events::Event* pEvt);

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_RENDER_needsRC);
	void do_PRE_RENDER_needsRC(PE::Events::Event* pEvt);
	
	PE::Handle m_hMovementSM;

	bool m_havePatrolWayPoint;
	char m_curPatrolWayPoint[32];
	States m_state;
};

};//namespace Components
};//namespace CharacterControl


#endif