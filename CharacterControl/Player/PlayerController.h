#ifndef _PLAYER_CONTROLLER_H_
#define _PLAYER_CONTROLLER_H_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Math/Vector3.h"
#include "PrimeEngine/Math/Matrix4x4.h"

#include "PrimeEngine/Scene/DebugRenderer.h"

namespace PE {
    namespace Events {
        struct EventQueueManager;
    }
}

namespace CharacterControl {
    namespace Components {

        // 输入控制器 - 负责采集输入
        struct PlayerGameControls : public PE::Components::Component
        {
            PE_DECLARE_CLASS(PlayerGameControls);
        public:
            PlayerGameControls(PE::GameContext& context, PE::MemoryArena arena, PE::Handle hMyself)
                : PE::Components::Component(context, arena, hMyself)
                , m_forward(0)
                , m_strafe(0)
                , m_rotate(0)
				, m_shoot(false)
                , m_frameTime(0)             
                , m_pQueueManager(nullptr)
                , m_enterPressed(false)
                , m_escapePressed(false)
            {
                PEINFO("PlayerGameControls constructor called\n");
            }

            virtual ~PlayerGameControls() { PEINFO("PlayerGameControls destructor called\n"); }

            // Component API
            PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
            virtual void do_UPDATE(PE::Events::Event* pEvt);
            virtual void addDefaultComponents();

            // 输入处理方法
            void handleKeyboardDebugInputEvents(PE::Events::Event* pEvt);
            void handleControllerDebugInputEvents(PE::Events::Event* pEvt);
            void handleIOSDebugInputEvents(PE::Events::Event* pEvt);

            PE::Events::EventQueueManager* m_pQueueManager;
            PrimitiveTypes::Float32 m_frameTime;

            // 控制值 - 供PlayerController读取
            float m_forward;   // W/S - 前后移动 (-1.0 to 1.0)
            float m_strafe;    // A/D - 左右平移 (-1.0 to 1.0)
            float m_rotate;    // 左右箭头 - 旋转 (-1.0 to 1.0)
            bool m_shoot;// 开火标志

			// 调试按键状态
            bool m_enterPressed;
            bool m_escapePressed;
        };

        // 主控制器 - 负责角色逻辑和状态
        struct PlayerController : public PE::Components::Component
        {
            PE_DECLARE_CLASS(PlayerController);

            enum GameState
            {
                GAME_STATE_MENU,
                GAME_STATE_PLAYING,
                GAME_STATE_PAUSED
            };

            PlayerController(PE::GameContext& context, PE::MemoryArena arena,
                PE::Handle myHandle, Vector3 spawnPos, float networkPingInterval);

            virtual void addDefaultComponents();

            PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
            virtual void do_UPDATE(PE::Events::Event* pEvt);
            PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_RENDER_needsRC);
            void do_PRE_RENDER_needsRC(PE::Events::Event* pEvt);

            void overrideTransform(Matrix4x4& t);
            void activate();

            // 状态变量
            Vector3 m_spawnPos;
            bool m_active;
            bool m_overriden;
            Matrix4x4 m_transformOverride;

            float m_networkPingTimer;
            float m_networkPingInterval;

            // 运动参数
            float m_moveSpeed;      // 移动速度
            float m_rotateSpeed;    // 旋转速度
            float m_currentRotation; // 当前Y轴旋转角度

			bool isMoving; // 检测是否在移动
			bool m_wasMoving; // 上一帧是否在移动
			bool m_isShooting; // 检测是否在开火

            // 射击参数
            float m_shootCooldown;
            float m_shootCooldownTimer;
            float m_shootDamage;
            float m_shootRange;

            // 射击方法
            void performShoot();

            //枪口火光
			float m_muzzleFlashTimer;
			bool m_showMuzzleFlash;

            // 游戏状态
            GameState m_gameState;
        };

        
    }; // namespace Components
}; // namespace CharacterControl

#endif