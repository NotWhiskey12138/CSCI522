#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "PlayerMovementSM.h"
#include "PlayerAnimationSM.h"
#include "Player.h"
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl{

// Events sent by behavior state machine (or other high level state machines)
// these are events that specify where a soldier should move
namespace Events{

PE_IMPLEMENT_CLASS1(PlayerMovementSM_Event_MOVE_TO, Event);

PlayerMovementSM_Event_MOVE_TO::PlayerMovementSM_Event_MOVE_TO(Vector3 targetPos /* = Vector3 */)
: m_targetPosition(targetPos)
{ }

PE_IMPLEMENT_CLASS1(PlayerMovementSM_Event_STOP, Event);

PE_IMPLEMENT_CLASS1(PlayerMovementSM_Event_TARGET_REACHED, Event);
}

namespace Components{

PE_IMPLEMENT_CLASS1(PlayerMovementSM, Component);


PlayerMovementSM::PlayerMovementSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself) 
: Component(context, arena, hMyself)
, m_state(STANDING)
{}

SceneNode *PlayerMovementSM::getParentsSceneNode()
{
	PE::Handle hParent = getFirstParentByType<Component>();
	if (hParent.isValid())
	{
		// see if parent has scene node component
		return hParent.getObject<Component>()->getFirstComponent<SceneNode>();
		
	}
	return NULL;
}

void PlayerMovementSM::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(PlayerMovementSM_Event_MOVE_TO, PlayerMovementSM::do_PlayerMovementSM_Event_MOVE_TO);
	PE_REGISTER_EVENT_HANDLER(PlayerMovementSM_Event_STOP, PlayerMovementSM::do_PlayerMovementSM_Event_STOP);
	
	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, PlayerMovementSM::do_UPDATE);
}

void PlayerMovementSM::do_PlayerMovementSM_Event_MOVE_TO(PE::Events::Event *pEvt)
{
	PlayerMovementSM_Event_MOVE_TO *pRealEvt = (PlayerMovementSM_Event_MOVE_TO *)(pEvt);
	
	// change state of this state machine
	m_state = WALKING_TO_TARGET;
	m_targetPostion = pRealEvt->m_targetPosition;

	// make sure the animations are playing
	
	PE::Handle h("PlayerAnimSM_Event_WALK", sizeof(PlayerAnimSM_Event_WALK));
	Events::PlayerAnimSM_Event_WALK *pOutEvt = new(h) PlayerAnimSM_Event_WALK();
	
	Player *pSol = getFirstParentByTypePtr<Player>();
	pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

	// release memory now that event is processed
	h.release();
}

void PlayerMovementSM::do_PlayerMovementSM_Event_STOP(PE::Events::Event *pEvt)
{
	Events::PlayerAnimSM_Event_STOP Evt;

	Player *pSol = getFirstParentByTypePtr<Player>();
	pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&Evt);
}

void PlayerMovementSM::do_UPDATE(PE::Events::Event *pEvt)
{
	if (m_state == WALKING_TO_TARGET)
	{
		// see if parent has scene node component
		SceneNode *pSN = getParentsSceneNode();
		if (pSN)
		{
			Vector3 curPos = pSN->m_base.getPos();
			float dsqr = (m_targetPostion - curPos).lengthSqr();

			bool reached = true;
			if (dsqr > 0.01f)
			{
				// not at the spot yet
				Event_UPDATE *pRealEvt = (Event_UPDATE *)(pEvt);
				static float speed = 1.4f;
				float allowedDisp = speed * pRealEvt->m_frameTime;

				Vector3 dir = (m_targetPostion - curPos);
				dir.normalize();
				float dist = sqrt(dsqr);
				if (dist > allowedDisp)
				{
					dist = allowedDisp; // can move up to allowedDisp
					reached = false; // not reaching destination yet
				}

				// instantaneous turn
				pSN->m_base.turnInDirection(dir, 3.1415f);
				pSN->m_base.setPos(curPos + dir * dist);
			}

			if (reached)
			{
				m_state = STANDING;
				
				// target has been reached. need to notify all same level state machines (components of parent)
				{
					PE::Handle h("PlayerMovementSM_Event_TARGET_REACHED", sizeof(PlayerMovementSM_Event_TARGET_REACHED));
					Events::PlayerMovementSM_Event_TARGET_REACHED *pOutEvt = new(h) PlayerMovementSM_Event_TARGET_REACHED();

					PE::Handle hParent = getFirstParentByType<Component>();
					if (hParent.isValid())
					{
						hParent.getObject<Component>()->handleEvent(pOutEvt);
					}
					
					// release memory now that event is processed
					h.release();
				}

				if (m_state == STANDING)
				{
					// no one has modified our state based on TARGET_REACHED callback
					// this means we are not going anywhere right now
					// so can send event to animation state machine to stop
					{
						Events::PlayerAnimSM_Event_STOP evt;
						
						Player *pSol = getFirstParentByTypePtr<Player>();
						pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&evt);
					}
				}
			}
		}
	}
}

}}




