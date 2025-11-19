#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "SoldierTPController.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "SoldierNPC.h"

#include "PrimeEngine/GameObjectModel/Camera.h"
#include "PrimeEngine/Scene/CameraManager.h"
#include "PrimeEngine/Logging/Log.h"

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl {
    namespace Components {

        PE_IMPLEMENT_CLASS1(SoldierTPController, Component);

        SoldierTPController::SoldierTPController(PE::GameContext& context,PE::MemoryArena arena,PE::Handle hMyself)
            : Component(context, arena, hMyself)
        {
        }

        void SoldierTPController::addDefaultComponents()
        {
            Component::addDefaultComponents();

            // 只注册 UPDATE，先不写实现
            PE_REGISTER_EVENT_HANDLER(Event_UPDATE, SoldierTPController::do_UPDATE);

            PE_REGISTER_EVENT_HANDLER(PE::Events::Event_PRE_RENDER_needsRC, SoldierTPController::do_PRE_RENDER_needsRC);

            //获得主相机
            Camera* pCam = CameraManager::Instance()->getActiveCamera();
			m_hCameraSceneNode = pCam->m_hCameraSceneNode;
        }

        // 先声明一个空的 do_UPDATE（注意签名必须对）
        void SoldierTPController::do_UPDATE(PE::Events::Event* pEvt)
        {
            PE::Events::Event_UPDATE* pRealEvent = (PE::Events::Event_UPDATE*)(pEvt);
            float dt = pRealEvent->m_frameTime;

            // 1. 找到父对象的 SoldierNPC
            CharacterControl::Components::SoldierNPC* pSol =
                getFirstParentByTypePtr<CharacterControl::Components::SoldierNPC>();

            if (!pSol)
                return; // 安全保护，避免空指针崩溃

            // 2. 从 SoldierNPC 上拿第一个 SceneNode（就是构造函数里 addComponent 的那个）
            PE::Handle hSN = pSol->getFirstComponentHandle<PE::Components::SceneNode>();
            if (hSN.isValid())
            {
                PE::Components::SceneNode* pSN = hSN.getObject<PE::Components::SceneNode>();

                // 3. 简单测试：一直往自己面朝的方向走
                Vector3 forward = pSN->m_base.getN();
                float speed = 2.0f;

                Vector3 pos = pSN->m_base.getPos();
                pos += forward * speed * dt;
                pSN->m_base.setPos(pos);

                CameraSceneNode* pCamSN = m_hCameraSceneNode.getObject<CameraSceneNode>();

                // 2. 士兵位置和方向
                Vector3 soldierPos = pSN->m_base.getPos();
                Vector3 forward2 = pSN->m_base.getN();
                Vector3 right = pSN->m_base.getU();
                Vector3 up = pSN->m_base.getV();

                // 3. 参数
                float camDist = 5.0f;
                float camHeight = 2.0f;

                // 4. 计算相机位置
                Vector3 camPos = soldierPos - forward2 * camDist + up * camHeight;

                // 5. 设置相机变换
                pCamSN->m_base.setPos(camPos);
                pCamSN->m_base.setU(right);
                pCamSN->m_base.setV(up);
                pCamSN->m_base.setN(forward2);

            }


        }

        void SoldierTPController::do_PRE_RENDER_needsRC(PE::Events::Event* pEvt)
        {
            PEINFO("SoldierTPController PRE_RENDER\n");
            // 这里可以添加需要渲染上下文的预渲染逻辑
            SoldierNPC* pSol = getFirstParentByTypePtr<SoldierNPC>();
            if (!pSol) return;

            PE::Handle hSN = pSol->getFirstComponentHandle<SceneNode>();
            if (!hSN.isValid()) return;

            SceneNode* pSN = hSN.getObject<SceneNode>();

            // 2. 士兵的变换
            Vector3 soldierPos = pSN->m_base.getPos();
            Vector3 forward = pSN->m_base.getN();
            Vector3 right = pSN->m_base.getU();
            Vector3 up = pSN->m_base.getV();

            // 3. 相机参数
            float camDist = 5.0f;
            float camHeight = 2.0f;

            Vector3 camPos = soldierPos - forward * camDist + up * camHeight;

            // 4. 取当前激活相机的 scene node
            Camera* pCam = CameraManager::Instance()->getActiveCamera(); // 具体函数名按 CameraManager.h 来
            CameraSceneNode* pCamSN = pCam->getCamSceneNode();

            // 5. 把相机矩阵强行设置成“跟随士兵”
            pCamSN->m_base.setPos(camPos);
            pCamSN->m_base.setU(right);
            pCamSN->m_base.setV(up);
            pCamSN->m_base.setN(forward);
		}
    } // namespace Components
} // namespace CharacterControl
