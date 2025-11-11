#ifndef _CHARACTER_CONTROL_PLAYER_EVENTS_H_
#define _CHARACTER_CONTROL_PLAYER_EVENTS_H_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Math/Vector3.h"

namespace CharacterControl {
    namespace Events {

        // 连续轴输入（移动/看向）
        struct Event_Player_MoveAxis : public PE::Events::Event
        {
            PE_DECLARE_CLASS(Event_Player_MoveAxis);
            Event_Player_MoveAxis(float x = 0.0f, float z = 0.0f) : moveX(x), moveZ(z) {}
            float moveX; // 左(-1)/右(+1)
            float moveZ; // 后(-1)/前(+1)
        };

        struct Event_Player_LookAxis : public PE::Events::Event
        {
            PE_DECLARE_CLASS(Event_Player_LookAxis);
            Event_Player_LookAxis(float dx = 0.0f, float dy = 0.0f) : deltaYaw(dx), deltaPitch(dy) {}
            float deltaYaw;
            float deltaPitch;
        };

        // 离散按钮（可扩展：跳跃、射击等）
        struct Event_Player_Action : public PE::Events::Event
        {
            PE_DECLARE_CLASS(Event_Player_Action);
            enum ActionType { ACTION_JUMP = 0, ACTION_FIRE = 1 };
            Event_Player_Action(ActionType t = ACTION_JUMP, bool pressed = false) : type(t), isPressed(pressed) {}
            ActionType type;
            bool isPressed;
        };

        // 通知/绑定相机跟随某个场景节点
        struct Event_BindCameraFollow : public PE::Events::Event
        {
            PE_DECLARE_CLASS(Event_BindCameraFollow);
            Event_BindCameraFollow(PE::Handle hTarget = PE::Handle()) : hFollowTarget(hTarget) {}
            PE::Handle hFollowTarget; // SceneNode 句柄
        };

    }
} // ns
#endif
