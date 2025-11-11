#include "PlayerCharacter.h"
#include "PrimeEngine/Scene/RootSceneNode.h"
#include "PlayerAnimSM.h"
#include "PlayerFSM.h"

using namespace PE;
using namespace PE::Components;

namespace CharacterControl {
    namespace Components {

        PE_IMPLEMENT_CLASS1(PlayerCharacter, PE::Components::Component);

        PlayerCharacter::PlayerCharacter(GameContext& ctx, MemoryArena arena, Handle hMyself)
            : Component(ctx, arena, hMyself) {
        }

        void PlayerCharacter::addDefaultComponents()
        {
            Component::addDefaultComponents();
        }

        void PlayerCharacter::initCharacter(const char* meshFile, const char* meshPkg,
            const char* skelFile, const char* animsetFile,
            const char* animsetCollection,
            const Vector3& pos, const Vector3& u,
            const Vector3& v, const Vector3& n,
            int threadMask)
        {
            // 创建根 SN
            {
                Handle hSN("SCENE_NODE", sizeof(SceneNode));
                SceneNode* pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
                pSN->addDefaultComponents();
                pSN->m_base.setPos(pos);
                pSN->m_base.setU(u); pSN->m_base.setV(v); pSN->m_base.setN(n);
                RootSceneNode::Instance()->addComponent(hSN);
                m_hRootSN = hSN;
                // 把 SN 作为“数据组件”挂到 PlayerCharacter（允许无事件）
                static int allowedEvts[] = { 0 };
                addComponent(hSN, &allowedEvts[0]);
            }

            // 旋转修正节点（可选：如果模型面向错误）
            {
                Handle hRotate("SCENE_NODE", sizeof(SceneNode));
                SceneNode* pRotate = new(hRotate) SceneNode(*m_pContext, m_arena, hRotate);
                pRotate->addDefaultComponents();
                // 例：转 180 度
                pRotate->m_base.turnLeft(3.1415f);
                m_hRotateSN = hRotate;
                m_hRootSN.getObject<SceneNode>()->addComponent(hRotate);
            }

            // 动画 SM
            {
                Handle hAnim("PlayerAnimSM", sizeof(PlayerAnimSM));
                PlayerAnimSM* pAnim = new(hAnim) PlayerAnimSM(*m_pContext, m_arena, hAnim);
                pAnim->addDefaultComponents();
                m_hAnimSM = hAnim;
            }

            // 骨骼 + 网格
            {
                Handle hSkel("SkeletonInstance", sizeof(SkeletonInstance));
                SkeletonInstance* pSkel = new(hSkel) SkeletonInstance(*m_pContext, m_arena, hSkel, m_hAnimSM);
                pSkel->addDefaultComponents();
                pSkel->initFromFiles(skelFile, animsetCollection, threadMask);
                pSkel->setAnimSet(animsetFile, animsetCollection);

                Handle hMesh("MeshInstance", sizeof(MeshInstance));
                MeshInstance* pMesh = new(hMesh) MeshInstance(*m_pContext, m_arena, hMesh);
                pMesh->addDefaultComponents();
                pMesh->initFromFile(meshFile, meshPkg, threadMask);

                pSkel->addComponent(hMesh);
                m_hMeshInst = hMesh; m_hSkelInst = hSkel;

                // 把骨骼挂到旋转节点
                m_hRotateSN.getObject<SceneNode>()->addComponent(hSkel);
            }

            // FSM
            {
                Handle hFSM("PlayerFSM", sizeof(PlayerFSM));
                PlayerFSM* pFSM = new(hFSM) PlayerFSM(*m_pContext, m_arena, hFSM);
                pFSM->addDefaultComponents();
                addComponent(hFSM); // 同级组件（可读到 SN）
                m_hFSM = hFSM;
            }
        }

    }
} // ns
