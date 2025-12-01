#ifndef _CHARACTER_VAMPIRE_MOVEMENT_SM_H_
#define _CHARACTER_VAMPIRE_MOVEMENT_SM_H_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Scene/SceneNode.h"

#include "CharacterControl/Events/Events.h"

namespace CharacterControl {

namespace Events
{
	//TODO: add events for vampire movement state machine
struct VampireMovementSM_Event_MOVE_TO : public PE::Events::Event {
	PE_DECLARE_CLASS(VampireMovementSM_Event_MOVE_TO);

	VampireMovementSM_Event_MOVE_TO(Vector3 targetPos = Vector3());

	Vector3 m_targetPosition;
};

struct VampireMovementSM_Event_STOP : public PE::Events::Event {
	PE_DECLARE_CLASS(VampireMovementSM_Event_STOP);

	VampireMovementSM_Event_STOP()
	{}

};

struct VampireMovementSM_Event_TARGET_REACHED : public PE::Events::Event {
	PE_DECLARE_CLASS(VampireMovementSM_Event_TARGET_REACHED);

	VampireMovementSM_Event_TARGET_REACHED()
	{}

};
};//namespace Events


namespace Components {

struct VampireMovementSM : public PE::Components::Component
{
	PE_DECLARE_CLASS(VampireMovementSM);

	enum States
	{
		STANDING,
		RUNNING_TO_TARGET,
		WALKING_TO_TARGET,
	};

	VampireMovementSM(PE::GameContext& context, PE::MemoryArena arena, PE::Handle hMyself);
	
	PE::Components::SceneNode *getParentsSceneNode();

	virtual void addDefaultComponents();

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_VampireMovementSM_Event_MOVE_TO);
	virtual void do_VampireMovementSM_Event_MOVE_TO(PE::Events::Event* pEvt);

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_VampireMovementSM_Event_STOP);
	virtual void do_VampireMovementSM_Event_STOP(PE::Events::Event* pEvt);

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
	virtual void do_UPDATE(PE::Events::Event* pEvt);

	PE::Handle m_hAnimationSM;

	States m_state;
	Vector3 m_targetPosition;
};

};//namespace Components
};//namespace CharacterControl


#endif