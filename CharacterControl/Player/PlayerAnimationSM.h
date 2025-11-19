#ifndef _PE_SOLDIER_NPC_ANIMATION_SM_H_
#define _PE_SOLDIER_NPC_ANIMATION_SM_H_


#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Scene/DefaultAnimationSM.h"


#include "../Events/Events.h"

namespace CharacterControl{

// events that can be sent to this state machine
namespace Events
{

// sent by movement state machine when a soldier has to stop
struct PlayerAnimSM_Event_STOP : public PE::Events::Event {
	PE_DECLARE_CLASS(PlayerAnimSM_Event_STOP);

	PlayerAnimSM_Event_STOP() {}
};

// sent by movement state machine when a soldier has to walk
struct PlayerAnimSM_Event_WALK : public PE::Events::Event {
	PE_DECLARE_CLASS(PlayerAnimSM_Event_WALK);

	PlayerAnimSM_Event_WALK() {}
};

// todo add events for shooting (sent by weapons state machine

};

namespace Components {

struct PlayerAnimationSM : public PE::Components::DefaultAnimationSM
{
	PE_DECLARE_CLASS(PlayerAnimationSM);
	
	enum AnimId
	{
		NONE = -1,
		STAND = 20,
		WALK = 5,
		RUN = 18,
		STAND_AIM = 1,
		STAND_SHOOT = 16,
	};

	PlayerAnimationSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself);

	// event handling
	virtual void addDefaultComponents();
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayerAnimSM_Event_STOP)
	virtual void do_PlayerAnimSM_Event_STOP(PE::Events::Event *pEvt);
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayerAnimSM_Event_WALK)
	virtual void do_PlayerAnimSM_Event_WALK(PE::Events::Event *pEvt);


	AnimId m_curId;
};

};
};


#endif


