#ifndef _SOLDIER_GAME_CONTROLS_H_
#define _SOLDIER_GAME_CONTROLS_H_

#include "PrimeEngine/Events/Component.h"

namespace CharacterControl {
    namespace Components {

        struct SoldierGameControls : public PE::Components::Component
        {
            PE_DECLARE_CLASS(SoldierGameControls);
        public:
            SoldierGameControls(PE::GameContext& context, PE::MemoryArena arena, PE::Handle hMyself)
                : PE::Components::Component(context, arena, hMyself)
            {
                m_forward = 0.0f;
                m_right = 0.0f;
                m_turn = 0.0f;
            }

            virtual ~SoldierGameControls() {}

            // 处理每一帧的更新
            PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
            virtual void do_UPDATE(PE::Events::Event* pEvt);

            virtual void addDefaultComponents();

            // 处理键盘事件
            void handleKeyboardDebugInputEvents(PE::Events::Event* pEvt);

            // 控制变量
            float m_forward; // 前后移动
            float m_right;   // 左右平移 (Strafe)
            float m_turn;    // 左右旋转
        };

    }; // namespace Components
}; // namespace CharacterControl

#endif