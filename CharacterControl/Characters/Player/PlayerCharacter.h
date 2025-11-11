#ifndef _CHARACTER_CONTROL_PLAYER_CHARACTER_H_
#define _CHARACTER_CONTROL_PLAYER_CHARACTER_H_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Scene/SkeletonInstance.h"
#include "PrimeEngine/Scene/MeshInstance.h"

namespace CharacterControl {
    namespace Components {

        struct PlayerCharacter : public PE::Components::Component
        {
            PE_DECLARE_CLASS(PlayerCharacter);

            PlayerCharacter(PE::GameContext& ctx, PE::MemoryArena arena, PE::Handle hMyself);

            virtual void addDefaultComponents();

            // 资源/初始变换
            void initCharacter(const PE::char* meshFile, const PE::char* meshPkg,
                const PE::char* skelFile, const PE::char* animsetFile,
                const PE::char* animsetCollection,
                const PE::Vector3& pos, const PE::Vector3& u,
                const PE::Vector3& v, const PE::Vector3& n,
                int threadMask);

            // 取根节点，给相机或外界绑定
            PE::Handle getSceneNodeHandle() const { return m_hRootSN; }

        private:
            PE::Handle m_hRootSN;        // 根 SceneNode
            PE::Handle m_hRotateSN;      // 用于修正导出朝向
            PE::Handle m_hSkelInst;      // SkeletonInstance
            PE::Handle m_hMeshInst;      // MeshInstance
            PE::Handle m_hAnimSM;        // PlayerAnimSM
            PE::Handle m_hFSM;           // PlayerFSM
        };

    }
} // ns
#endif
