#ifndef _CHARACTER_VAMPIRE_ANIMATION_
#define _CHARACTER_VAMPIRE_ANIMATION_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Scene/DefaultAnimationSM.h"

#include "CharacterControl/Events/Events.h"

namespace CharacterControl {

namespace Events {

	//TODO:add events for vampire animation state machine
	struct VampireAnimSM_Event_STOP : public PE::Events::Event {
		PE_DECLARE_CLASS(VampireAnimSM_Event_STOP);

		VampireAnimSM_Event_STOP() {}
	};

	struct VampireAnimSM_Event_WALK : public PE::Events::Event {
		PE_DECLARE_CLASS(VampireAnimSM_Event_WALK);

		VampireAnimSM_Event_WALK() {}
	};

};//namespace Events


namespace Components {

struct VampireAnimationSM : public PE::Components::DefaultAnimationSM
{
	PE_DECLARE_CLASS(VampireAnimationSM);

	enum AnimId
	{
		NONE = -1,
		STAND = 0,
		WALK = 1,
		RUN = 2,
		ATTACK = 3,
		DIE = 4
	};

	VampireAnimationSM(PE::GameContext& context, PE::MemoryArena arena, PE::Handle hMyself);

	// event handling
	virtual void addDefaultComponents();
	//TODO: add Event Handlers
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_VampireAnimSM_Event_STOP);
	virtual void do_VampireAnimSM_Event_STOP(PE::Events::Event* pEvt);
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_VampireAnimSM_Event_WALK);
	virtual void do_VampireAnimSM_Event_WALK(PE::Events::Event* pEvt);

	AnimId m_curId;
};

};//namespace Components
};//namespace CharacterControl


#endif