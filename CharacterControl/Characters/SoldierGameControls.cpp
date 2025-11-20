#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "SoldierGameControls.h"
#include "PrimeEngine/Events/StandardGameEvents.h"
#include "PrimeEngine/Events/StandardKeyboardEvents.h"
#include "PrimeEngine/Events/EventQueueManager.h"

using namespace PE::Components;
using namespace PE::Events;

namespace CharacterControl {
    namespace Components {

        PE_IMPLEMENT_CLASS1(SoldierGameControls, PE::Components::Component);

        void SoldierGameControls::addDefaultComponents()
        {
            Component::addDefaultComponents();
            PE_REGISTER_EVENT_HANDLER(Event_UPDATE, SoldierGameControls::do_UPDATE);
        }

        void SoldierGameControls::do_UPDATE(PE::Events::Event* pEvt)
        {
            // 每一帧重置变量，只有按住键时才有值
            m_forward = 0.0f;
            m_right = 0.0f;
            m_turn = 0.0f;

            PE::Handle iqh = PE::Events::EventQueueManager::Instance()->getEventQueueHandle("input");

            // 处理输入队列
            while (!iqh.getObject<PE::Events::EventQueue>()->empty())
            {
                PE::Events::Event* pInputEvt = iqh.getObject<PE::Events::EventQueue>()->getFront();
                handleKeyboardDebugInputEvents(pInputEvt);
                iqh.getObject<PE::Events::EventQueue>()->destroyFront();
            }

            // 清理队列
            iqh.getObject<PE::Events::EventQueue>()->destroy();
        }

        void SoldierGameControls::handleKeyboardDebugInputEvents(Event* pEvt)
        {
            float moveSpeed = 5.0f;
            float turnSpeed = 3.0f;

            // --- 处理前后移动 (W / S) ---
            if (Event_KEY_W_HELD::GetClassId() == pEvt->getClassId())
            {
                m_forward = moveSpeed; // 正数向前
            }
            else if (Event_KEY_S_HELD::GetClassId() == pEvt->getClassId())
            {
                m_forward = -moveSpeed; // 负数向后
            }

            // --- 处理左右平移 (A / D) ---
            // 注意：这里使用一个新的 if，不要用 else if 连着上面的 W/S
            // 这样可以确保如果你同时按 W 和 D，两个都会被处理
            if (Event_KEY_A_HELD::GetClassId() == pEvt->getClassId())
            {
                m_right = -moveSpeed; // 负数向左
            }
            else if (Event_KEY_D_HELD::GetClassId() == pEvt->getClassId())
            {
                m_right = moveSpeed; // 正数向右
            }

            // --- 处理左右旋转 (Left / Right Arrow) ---
            if (Event_KEY_LEFT_HELD::GetClassId() == pEvt->getClassId())
            {
                m_turn = turnSpeed;
            }
            else if (Event_KEY_RIGHT_HELD::GetClassId() == pEvt->getClassId())
            {
                m_turn = -turnSpeed;
            }

            Component::handleEvent(pEvt);
        }

    }
}