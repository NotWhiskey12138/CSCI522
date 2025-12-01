#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "Vampire.h"
#include "VampireAnimationSM.h"
#include "VampireMovementSM.h"

using namespace PE::Events;
using namespace PE::Components;
using namespace CharacterControl::Events;

namespace CharacterControl {

namespace Events {

	PE_IMPLEMENT_CLASS1(VampireMovementSM_Event_MOVE_TO, Event);

	VampireMovementSM_Event_MOVE_TO::VampireMovementSM_Event_MOVE_TO(Vector3 targetPos /* = Vector3 */)
		: m_targetPosition(targetPos)
	{
	}

	PE_IMPLEMENT_CLASS1(VampireMovementSM_Event_STOP, Event);

	PE_IMPLEMENT_CLASS1(VampireMovementSM_Event_TARGET_REACHED, Event);

};


namespace Components {

PE_IMPLEMENT_CLASS1(VampireMovementSM, Component);

VampireMovementSM::VampireMovementSM(PE::GameContext& context, PE::MemoryArena arena, PE::Handle hMyself)
	: Component(context, arena, hMyself)
	, m_state(STANDING)
{}

SceneNode* VampireMovementSM::getParentsSceneNode()
{
	PE::Handle hParent = getFirstParentByType<Component>();
	if (hParent.isValid())
	{
		// see if parent has scene node component
		return hParent.getObject<Component>()->getFirstComponent<SceneNode>();
	}
	return NULL;
}

void VampireMovementSM::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(VampireMovementSM_Event_MOVE_TO, VampireMovementSM::do_VampireMovementSM_Event_MOVE_TO);
	PE_REGISTER_EVENT_HANDLER(VampireMovementSM_Event_STOP, VampireMovementSM::do_VampireMovementSM_Event_STOP);

	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, VampireMovementSM::do_UPDATE);
}

void VampireMovementSM::do_VampireMovementSM_Event_MOVE_TO(PE::Events::Event* pEvt)
{
	VampireMovementSM_Event_MOVE_TO* pRealEvt = (VampireMovementSM_Event_MOVE_TO*)(pEvt);

	//改变状态
	m_state = WALKING_TO_TARGET;
	m_targetPosition = pRealEvt->m_targetPosition;

	//通知动画状态机

	PE::Handle h("VampireAnimSM_Event_WALK", sizeof(VampireAnimSM_Event_WALK));
	Events::VampireAnimSM_Event_WALK* pOutEvt = new(h) VampireAnimSM_Event_WALK();

	//获取父对象并发送事件
	Vampire* pSol = getFirstParentByTypePtr<Vampire>();
	pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

	h.release();
}

void VampireMovementSM::do_VampireMovementSM_Event_STOP(PE::Events::Event* pEvt)
{
	//创建停止事件
	Events::VampireAnimSM_Event_STOP Evt;

	//通知动画状态机停止
	Vampire* pSol = getFirstParentByTypePtr<Vampire>();
	pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&Evt);
}

void VampireMovementSM::do_UPDATE(PE::Events::Event* pEvt) 
{
	if (m_state == WALKING_TO_TARGET)
	{
		//查看父类是否有场景节点组件
		SceneNode* pSN = getParentsSceneNode();
		if (pSN) 
		{
			//获取当前位置
			Vector3 curPos = pSN->m_base.getPos();
			float dsqr = (m_targetPosition - curPos).lengthSqr();

			//计算是否到达目标位置
			bool reached = true;
			if(dsqr > 0.01f) 
			{
				//还没到达目标位置
				Event_UPDATE* pRealEvt = (Event_UPDATE*)(pEvt);
				static float speed = 1.4f;
				float allowedDisp = speed * pRealEvt->m_frameTime;

				//计算移动方向和距离
				Vector3 dir = (m_targetPosition - curPos);
				dir.normalize();
				float dist = sqrt(dsqr);
				if (dist > allowedDisp) 
				{
					dist = allowedDisp; //可以移动的最大距离
					reached = false; //还没到达目的地
				}

				//瞬间转向
				pSN->m_base.turnInDirection(dir, 3.1415f);
				pSN->m_base.setPos(curPos + dir * dist);
			}

			if(reached) 
			{
				//到达目标位置，改变状态并通知动画状态机
				m_state = STANDING;

				{
					//通知动画状态机
					PE::Handle h("VampireMovementSM_Event_TARGET_REACHED", sizeof(VampireMovementSM_Event_TARGET_REACHED));
					Events::VampireMovementSM_Event_TARGET_REACHED* pOutEvt = new(h) VampireMovementSM_Event_TARGET_REACHED();

					///获取父对象并发送事件
					PE::Handle hParent = getFirstParentByType<Component>();
					if(hParent.isValid()) 
					{
						hParent.getObject<Component>()->handleEvent(pOutEvt);
					}

					//释放内存
					h.release();
				}

				if(m_state == STANDING) 
				{
					//创建停止事件
					Events::VampireAnimSM_Event_STOP Evt;

					//通知动画状态机停止
					Vampire* pSol = getFirstParentByTypePtr<Vampire>();
					pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&Evt);
				}
			}
		}

	}

}
};//namespace Components
};//namespace CharacterControl