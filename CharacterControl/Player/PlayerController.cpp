#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"
#include "PlayerController.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Scene/CameraSceneNode.h"
#include "PrimeEngine/Scene/CameraManager.h"
#include "CharacterControl/CharacterControlContext.h"
#include "PrimeEngine/Events/StandardKeyboardEvents.h"
#include "PrimeEngine/Events/StandardIOSEvents.h"
#include "PrimeEngine/Events/StandardControllerEvents.h"
#include "PrimeEngine/Events/EventQueueManager.h"
#include "PrimeEngine/GameObjectModel/DefaultGameControls/DefaultGameControls.h"

using namespace PE::Components;
using namespace PE::Events;

namespace CharacterControl {
    namespace Components {

        // ==================== PlayerGameControls ====================

        PE_IMPLEMENT_CLASS1(PlayerGameControls, PE::Components::Component);

        void PlayerGameControls::addDefaultComponents()
        {
            Component::addDefaultComponents();
            PE_REGISTER_EVENT_HANDLER(Event_UPDATE, PlayerGameControls::do_UPDATE);
        }

        void PlayerGameControls::do_UPDATE(PE::Events::Event* pEvt)
        {
            // 重置控制值
            m_forward = 0;
            m_strafe = 0;
            m_rotate = 0;

            // 处理输入队列
            PE::Handle iqh = PE::Events::EventQueueManager::Instance()->getEventQueueHandle("input");

            m_frameTime = ((Event_UPDATE*)(pEvt))->m_frameTime;

            while (!iqh.getObject<PE::Events::EventQueue>()->empty())
            {
                PE::Events::Event* pInputEvt = iqh.getObject<PE::Events::EventQueue>()->getFront();

                handleKeyboardDebugInputEvents(pInputEvt);
                handleControllerDebugInputEvents(pInputEvt);
                handleIOSDebugInputEvents(pInputEvt);

                iqh.getObject<PE::Events::EventQueue>()->destroyFront();
            }

            iqh.getObject<PE::Events::EventQueue>()->destroy();
        }

        void PlayerGameControls::handleKeyboardDebugInputEvents(Event* pEvt)
        {
            // WASD 移动
            if (Event_KEY_W_HELD::GetClassId() == pEvt->getClassId())
            {
                m_forward = 1.0f;
            }
            else if (Event_KEY_S_HELD::GetClassId() == pEvt->getClassId())
            {
                m_forward = -1.0f;
            }
            else if (Event_KEY_A_HELD::GetClassId() == pEvt->getClassId())
            {
                m_strafe = -1.0f;
            }
            else if (Event_KEY_D_HELD::GetClassId() == pEvt->getClassId())
            {
                m_strafe = 1.0f;
            }
            // 箭头键旋转
            else if (Event_KEY_LEFT_HELD::GetClassId() == pEvt->getClassId())
            {
                m_rotate = 1.0f;  // 左转
            }
            else if (Event_KEY_RIGHT_HELD::GetClassId() == pEvt->getClassId())
            {
                m_rotate = -1.0f; // 右转
            }
        }

        void PlayerGameControls::handleControllerDebugInputEvents(Event* pEvt)
        {
            if (Event_ANALOG_L_THUMB_MOVE::GetClassId() == pEvt->getClassId())
            {
                Event_ANALOG_L_THUMB_MOVE* pRealEvent = (Event_ANALOG_L_THUMB_MOVE*)(pEvt);

                m_forward = pRealEvent->m_absPosition.getY();
                m_strafe = pRealEvent->m_absPosition.getX();
            }
            else if (Event_ANALOG_R_THUMB_MOVE::GetClassId() == pEvt->getClassId())
            {
                Event_ANALOG_R_THUMB_MOVE* pRealEvent = (Event_ANALOG_R_THUMB_MOVE*)(pEvt);

                m_rotate = pRealEvent->m_absPosition.getX();
            }
        }

        void PlayerGameControls::handleIOSDebugInputEvents(Event* pEvt)
        {
#if APIABSTRACTION_IOS
            // iOS触摸控制实现
            if (Event_IOS_TOUCH_MOVED::GetClassId() == pEvt->getClassId())
            {
                Event_IOS_TOUCH_MOVED* pRealEvent = (Event_IOS_TOUCH_MOVED*)(pEvt);

                if (pRealEvent->touchesCount > 1)
                {
                    m_forward = -pRealEvent->m_normalized_dy * 10.0f;
                }
                else
                {
                    m_rotate = pRealEvent->m_normalized_dx * 10.0f;
                }
            }
#endif
        }

        // ==================== PlayerController ====================

        PE_IMPLEMENT_CLASS1(PlayerController, Component);

        PlayerController::PlayerController(PE::GameContext& context, PE::MemoryArena arena,
            PE::Handle myHandle, Vector3 spawnPos, float networkPingInterval)
            : Component(context, arena, myHandle)
            , m_spawnPos(spawnPos)
            , m_active(false)
            , m_overriden(false)
            , m_networkPingTimer(0)
            , m_networkPingInterval(networkPingInterval)
            , m_moveSpeed(5.0f)        // 5 units/second
            , m_rotateSpeed(3.14159f)  // 180 degrees/second
            , m_currentRotation(0)
        {
        }

        void PlayerController::addDefaultComponents()
        {
            Component::addDefaultComponents();
            PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, PlayerController::do_UPDATE);
        }

        void PlayerController::do_UPDATE(PE::Events::Event* pEvt)
        {
            PE::Events::Event_UPDATE* pRealEvt = (PE::Events::Event_UPDATE*)(pEvt);

            // 获取场景节点
            PE::Handle hFirstSN = getFirstComponentHandle<SceneNode>();
            if (!hFirstSN.isValid())
            {
                assert(!"PlayerController must have a SceneNode component");
                return;
            }

            SceneNode* pFirstSN = hFirstSN.getObject<SceneNode>();

            if (m_active && !m_overriden)
            {
                // 获取输入控制值
                CharacterControlContext* pCtx = m_pContext->get<CharacterControlContext>();
                PlayerGameControls* pCtrl = pCtx->getPlayerGameControls();

                float deltaTime = pRealEvt->m_frameTime;

                // 1. 处理旋转（箭头键）
                if (fabs(pCtrl->m_rotate) > 0.1f) // 死区
                {
                    m_currentRotation += pCtrl->m_rotate * m_rotateSpeed * deltaTime;
                }

                // 2. 处理移动（WASD）
                Vector3 localMove(pCtrl->m_strafe, 0, pCtrl->m_forward);

                if (localMove.lengthSqr() > 0.01f) // 如果有移动输入
                {
                    // 归一化，避免对角线移动过快
                    localMove.normalize();
                    localMove *= m_moveSpeed * deltaTime;

                    // 将局部移动转换到世界空间（根据当前旋转）
                    Matrix4x4 rotationMatrix;
                    rotationMatrix.loadIdentity();
                    rotationMatrix.setU(Vector3(cos(m_currentRotation), 0, sin(m_currentRotation)));
                    rotationMatrix.setN(Vector3(-sin(m_currentRotation), 0, cos(m_currentRotation)));

                    Vector3 worldMove = rotationMatrix * localMove;

                    // 更新位置
                    Vector3 currentPos = pFirstSN->m_base.getPos();
                    pFirstSN->m_base.setPos(currentPos + worldMove);
                }

                // 3. 更新旋转矩阵
                pFirstSN->m_base.setU(Vector3(cos(m_currentRotation), 0, sin(m_currentRotation)));
                pFirstSN->m_base.setN(Vector3(-sin(m_currentRotation), 0, cos(m_currentRotation)));
                pFirstSN->m_base.setV(Vector3(0, 1, 0));

                // 4. 网络同步计时
                m_networkPingTimer += deltaTime;
                if (m_networkPingTimer > m_networkPingInterval)
                {
                    // TODO: 发送网络同步事件
                    m_networkPingTimer = 0.0f;
                }
            }
            else if (m_overriden)
            {
                // 网络覆盖模式
                pFirstSN->m_base = m_transformOverride;
            }

            // 更新第三人称摄像机
            SceneNode* pCamParent = pFirstSN->getFirstComponent<SceneNode>();
            if (pCamParent)
            {
                if (CameraSceneNode* pCamSN = pCamParent->getFirstComponent<CameraSceneNode>())
                {
                    // 摄像机位置：角色后方偏上
                    pCamSN->m_base.loadIdentity();
                    pCamSN->m_base.setPos(Vector3(0, 3.0f, -5.0f)); // 后方5米，上方3米
                }
            }
        }

        void PlayerController::overrideTransform(Matrix4x4& t)
        {
            m_overriden = true;
            m_transformOverride = t;
        }

        void PlayerController::activate()
        {
            m_active = true;

            // 获取场景节点
            PE::Handle hFirstSN = getFirstComponentHandle<SceneNode>();
            if (!hFirstSN.isValid())
            {
                assert(!"PlayerController must have a SceneNode component");
                return;
            }
            SceneNode* pFirstSN = hFirstSN.getObject<SceneNode>();

            // 创建摄像机父节点（用于独立控制摄像机）
            PE::Handle hCamParent("SceneNode", sizeof(SceneNode));
            SceneNode* pCamParent = new(hCamParent) SceneNode(*m_pContext, m_arena, hCamParent);
            pCamParent->addDefaultComponents();
            pFirstSN->addComponent(hCamParent);

            // 创建摄像机
            PE::Handle hCamera("Camera", sizeof(Camera));
            Camera* pCamera = new(hCamera) Camera(*m_pContext, m_arena, hCamera, hCamParent);
            pCamera->addDefaultComponents();

            // 设置为第三人称摄像机
            CameraManager::Instance()->setCamera(CameraManager::VEHICLE, hCamera);
            CameraManager::Instance()->selectActiveCamera(CameraManager::VEHICLE);

            // 禁用默认控制
            m_pContext->getDefaultGameControls()->setEnabled(false);

            // 禁用其他载具控制
            //m_pContext->get<CharacterControlContext>()->getTankGameControls()->setEnabled(false);
            //m_pContext->get<CharacterControlContext>()->getSpaceShipGameControls()->setEnabled(false);

            // 启用玩家控制
            m_pContext->get<CharacterControlContext>()->getPlayerGameControls()->setEnabled(true);
        }

    }
}