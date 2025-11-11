#ifndef _CHARACTER_CONTROL_CAMERA_FOLLOW_H_
#define _CHARACTER_CONTROL_CAMERA_FOLLOW_H_

#include "PrimeEngine/Events/Component.h"
#include "PlayerEvents.h"

namespace CharacterControl {
    namespace Components {

        struct CameraFollow : public PE::Components::Component
        {
            PE_DECLARE_CLASS(CameraFollow);

            CameraFollow(PE::GameContext& ctx, PE::MemoryArena arena, PE::Handle hMyself);

            virtual void addDefaultComponents();

            void do_UPDATE(PE::Events::Event* pEvt);
            void do_Bind(PE::Events::Event* pEvt);

            void setOffsets(float dist, float height) { m_dist = dist; m_height = height; }

        private:
            PE::Handle m_hCameraSN;    // 自己的 SceneNode（相机）
            PE::Handle m_hFollowSN;    // 目标 SceneNode（玩家）
            float m_dist;              // 距离
            float m_height;            // 高度
        };

    }
} // ns
#endif
