#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/DebugRenderer.h"
#include "CharacterControl/ClientGameObjectManagerAddon.h"
#include "CharacterControl/CharacterControlContext.h"
#include "VampireMovementSM.h"
#include "VampireAnimationSM.h"
#include "VampireBehaviorSM.h"
#include "Vampire.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Render/IRenderer.h"

using namespace PE::Events;
using namespace PE::Components;
using namespace CharacterControl::Events;

namespace CharacterControl {

namespace Components {

PE_IMPLEMENT_CLASS1(VampireBehaviorSM, Component);

VampireBehaviorSM::VampireBehaviorSM(PE::GameContext& context, PE::MemoryArena arena, PE::Handle hMyself, PE::Handle hMovementSM)
	: Component(context, arena, hMyself)
	, m_hMovementSM(hMovementSM)
{

}

void VampireBehaviorSM::start()
{
	// 初始化状态机的状态
	if (m_havePatrolWayPoint)
	{
		m_state = WAITING_FOR_WAYPOINT; // will update on next do_UPDATE()
	}
	else
	{
		m_state = IDLE; // stand in place
		PE::Handle h("VampireMovementSM_Event_STOP", sizeof(VampireMovementSM_Event_STOP));
		VampireMovementSM_Event_STOP* pEvt = new(h) VampireMovementSM_Event_STOP();
		m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
		// release memory now that event is processed
		h.release();
	}
}

void VampireBehaviorSM::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(VampireMovementSM_Event_TARGET_REACHED, VampireBehaviorSM::do_VampireMovementSM_Event_TARGET_REACHED);
	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, VampireBehaviorSM::do_UPDATE);

	PE_REGISTER_EVENT_HANDLER(Event_PRE_RENDER_needsRC, VampireBehaviorSM::do_PRE_RENDER_needsRC);
};

void VampireBehaviorSM::do_VampireMovementSM_Event_TARGET_REACHED(PE::Events::Event* pEvt)
{
	PEINFO("VampireBehaviorSM::do_VampireMovementSM_Event_TARGET_REACHED\n");

	if (m_state == PATROLLING_WAYPOINTS)
	{
		ClientGameObjectManagerAddon* pGameObjectManagerAddon = (ClientGameObjectManagerAddon*)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
		if (pGameObjectManagerAddon) {

			//找到下一个巡逻点
			WayPoint* pWP = pGameObjectManagerAddon->getWayPoint(m_curPatrolWayPoint);
			if (pWP && StringOps::length(pWP->m_nextWayPointName) > 0)
			{
				pWP = pGameObjectManagerAddon->getWayPoint(pWP->m_nextWayPointName);
				if (pWP)
				{
					StringOps::writeToString(pWP->m_name, m_curPatrolWayPoint, 32);

					m_state = PATROLLING_WAYPOINTS;
					PE::Handle h("VampireMovementSM_Event_MOVE_TO", sizeof(VampireMovementSM_Event_MOVE_TO));
					Events::VampireMovementSM_Event_MOVE_TO* pEvt = new(h) VampireMovementSM_Event_MOVE_TO(pWP->m_base.getPos());

					m_hMovementSM.getObject<Component>()->handleEvent(pEvt);

					// release memory now that event is processed
					h.release();
				}
			}
			else
			{
				m_state = IDLE;
			}
		}
	}
}

void VampireBehaviorSM::do_PRE_RENDER_needsRC(PE::Events::Event* pEvt)
{
	Event_PRE_RENDER_needsRC* pRealEvent = (Event_PRE_RENDER_needsRC*)(pEvt);
	if (m_havePatrolWayPoint) 
	{
		char buf[80];
		sprintf(buf, "Patrol Waypoint: %s", m_curPatrolWayPoint);

		//获取父类对象
		Vampire* pVampire = getFirstParentByTypePtr<Vampire>();
		PE::Handle hVampireSceneNode = pVampire->getFirstComponentHandle<PE::Components::SceneNode>();
		Matrix4x4 base = hVampireSceneNode.getObject<PE::Components::SceneNode>()->m_worldTransform;

		// 绘制调试文本
		DebugRenderer::Instance()->createTextMesh(
			buf, false, false, true, false, 0,
			base.getPos(), 0.1f, pRealEvent->m_threadOwnershipMask);
		
		{
			bool sent = false;
			ClientGameObjectManagerAddon* pGameObjectManagerAddon = (ClientGameObjectManagerAddon*)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
			if (pGameObjectManagerAddon) 
			{
				WayPoint* pWP = pGameObjectManagerAddon->getWayPoint(m_curPatrolWayPoint);
				if (pWP)
				{
					Vector3 target = pWP->m_base.getPos();
					Vector3 pos = base.getPos();
					Vector3 color(1.0f, 1.0f, 0);
					Vector3 linepts[] = { pos, color, target, color };

					DebugRenderer::Instance()->createLineMesh(true, base, &linepts[0].m_x, 2, 0); 
					sent = true;
				}
			}
			if (!sent)
				DebugRenderer::Instance()->createLineMesh(true, base, NULL, 0, 0);
		}
	}
}

void VampireBehaviorSM::do_UPDATE(PE::Events::Event* pEvt) 
{
	if (m_state = WAITING_FOR_WAYPOINT) {
		ClientGameObjectManagerAddon* pGameObjectManagerAddon = (ClientGameObjectManagerAddon*)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
		if (pGameObjectManagerAddon) 
		{
			WayPoint* pWP = pGameObjectManagerAddon->getWayPoint(m_curPatrolWayPoint);
			if (pWP) 
			{
				m_state= PATROLLING_WAYPOINTS;
				PE::Handle h("VampireMovementSM_Event_MOVE_TO", sizeof(VampireMovementSM_Event_MOVE_TO));
				Events::VampireMovementSM_Event_MOVE_TO* pEvt = new(h) VampireMovementSM_Event_MOVE_TO(pWP->m_base.getPos());

				m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
				// release memory now that event is processed
				h.release();
			}
		}
	}

	else
	{
		m_state = IDLE;

		PE::Handle h("VampireMovementSM_Event_STOP", sizeof(VampireMovementSM_Event_STOP));
		VampireMovementSM_Event_STOP* pEvt = new(h) VampireMovementSM_Event_STOP();

		m_hMovementSM.getObject<Component>()->handleEvent(pEvt);

		h.release();
	}
}

};//namespace Components
};//namespace CharacterControl