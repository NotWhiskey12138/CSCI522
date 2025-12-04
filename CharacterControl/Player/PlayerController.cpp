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
#include "PrimeEngine/Scene/SkeletonInstance.h"
#include "CharacterControl/Events/Events.h"
#include "CharacterControl/Characters/SoldierNPCAnimationSM.h"

#include "CharacterControl/Characters/SoldierNPC.h"
#include "CharacterControl/ClientGameObjectManagerAddon.h"
#include "PrimeEngine/Scene/DebugRenderer.h"
#include "PrimeEngine/Events/StandardEvents.h"


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
			m_shoot = false;

			// 重置调试按键状态
            m_enterPressed = false;
            m_escapePressed = false;

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
                m_forward = -1.0f;
               /* PEINFO("W press\n");*/
            }
            else if (Event_KEY_S_HELD::GetClassId() == pEvt->getClassId())
            {
                m_forward = 1.0f;
            }
            else if (Event_KEY_A_HELD::GetClassId() == pEvt->getClassId())
            {
                m_strafe = 1.0f;
            }
            else if (Event_KEY_D_HELD::GetClassId() == pEvt->getClassId())
            {
                m_strafe = -1.0f;
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
            else if (Event_KEY_SPACE_HELD::GetClassId() == pEvt->getClassId()) 
            {
				m_shoot = true; // 开火
			/*	PEINFO("Space pressed! m_shoot = true\n");*/
            }
            else if (Event_KEY_ENTER_DOWN::GetClassId() == pEvt->getClassId())
            {
                m_enterPressed = true;
            }
            else if (Event_KEY_ESCAPE_DOWN::GetClassId() == pEvt->getClassId())
            {
                m_escapePressed = true;
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
			, m_wasMoving(false)
			, m_isShooting(false)
            , m_shootCooldown(0.3f)
            , m_shootCooldownTimer(0)
            , m_shootDamage(25.0f)
            , m_shootRange(100.0f)
			, m_muzzleFlashTimer(0)
			, m_showMuzzleFlash(false)
            , m_gameState(GAME_STATE_MENU)
        {
        }

        void PlayerController::addDefaultComponents()
        {
            Component::addDefaultComponents();
            PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, PlayerController::do_UPDATE);
            PE_REGISTER_EVENT_HANDLER(PE::Events::Event_PRE_RENDER_needsRC, PlayerController::do_PRE_RENDER_needsRC);
        }

        void PlayerController::do_UPDATE(PE::Events::Event* pEvt)
        {
            PE::Events::Event_UPDATE* pRealEvt = (PE::Events::Event_UPDATE*)(pEvt);

            // 获取输入控制值
            CharacterControlContext* pCtx = m_pContext->get<CharacterControlContext>();
            PlayerGameControls* pCtrl = pCtx->getPlayerGameControls();

            // === 处理游戏状态 ===
            if (m_gameState == GAME_STATE_MENU)
            {
                // 按 Enter 开始游戏
                if (pCtrl->m_enterPressed)
                {
                    m_gameState = GAME_STATE_PLAYING;
                    PEINFO("Game Started!\n");
                }

                // 不处理游戏逻辑
                return;
            }
            else if (m_gameState == GAME_STATE_PLAYING)
            {
                // 按 Escape 退出
                if (pCtrl->m_escapePressed)
                {
                    PEINFO("Game Exit!\n");
                    exit(0);
                }
            }

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
                

                float deltaTime = pRealEvt->m_frameTime;

                // === 添加射击逻辑 ===
                if (m_shootCooldownTimer > 0)
                {
                    m_shootCooldownTimer -= deltaTime;
                }

                if (pCtrl->m_shoot && m_shootCooldownTimer <= 0)
                {
                    performShoot();
                    m_shootCooldownTimer = m_shootCooldown;
                }

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

                    isMoving = true;
                }
                else {
                    isMoving = false;
				}

                // 3. 更新旋转矩阵
                pFirstSN->m_base.setU(Vector3(cos(m_currentRotation), 0, sin(m_currentRotation)));
                pFirstSN->m_base.setN(Vector3(-sin(m_currentRotation), 0, cos(m_currentRotation)));
                pFirstSN->m_base.setV(Vector3(0, 1, 0));

                bool wantShoot = pCtrl->m_shoot;

                SkeletonInstance* pSkelInst = pFirstSN->getFirstComponent<SkeletonInstance>();
                if (pSkelInst)
                {
                    // 射击状态变化
                    if (wantShoot != m_isShooting)
                    {
                        m_isShooting = wantShoot;

                        if (m_isShooting)
                        {
                            CharacterControl::Events::SoldierNPCAnimSM_Event_SHOOT shootEvt;
                            pSkelInst->handleEvent(&shootEvt);
                        }
                        else if (isMoving)
                        {
                            CharacterControl::Events::SoldierNPCAnimSM_Event_WALK walkEvt;
                            pSkelInst->handleEvent(&walkEvt);
                        }
                        else
                        {
                            CharacterControl::Events::SoldierNPCAnimSM_Event_STOP stopEvt;
                            pSkelInst->handleEvent(&stopEvt);
                        }
                    }
                    // 移动状态变化（只在非射击时处理）
                    else if (!m_isShooting && isMoving != m_wasMoving)
                    {
                        if (isMoving)
                        {
                            CharacterControl::Events::SoldierNPCAnimSM_Event_WALK walkEvt;
                            pSkelInst->handleEvent(&walkEvt);
                        }
                        else
                        {
                            CharacterControl::Events::SoldierNPCAnimSM_Event_STOP stopEvt;
                            pSkelInst->handleEvent(&stopEvt);
                        }
                        m_wasMoving = isMoving;
                    }
                }

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
                    pCamSN->m_base.setPos(Vector3(0, 3.0f, 5.0f)); // 后方5米，上方3米

                    // 让摄像机朝向 -Z（看向角色）
                    pCamSN->m_base.setU(Vector3(-1, 0, 0));  // 右
                    pCamSN->m_base.setV(Vector3(0, 1, 0));   // 上
                    pCamSN->m_base.setN(Vector3(0, 0, -1));  // 前（朝向角色）
                }
            }

            // === 枪口火光渲染 ===
            if (m_showMuzzleFlash)
            {
                m_muzzleFlashTimer -= pRealEvt->m_frameTime;

                if (m_muzzleFlashTimer <= 0)
                {
                    m_showMuzzleFlash = false;
                }
                else
                {
                    // 获取枪口位置（角色前方）
                    PE::Handle hFirstSN = getFirstComponentHandle<SceneNode>();
                    if (hFirstSN.isValid())
                    {
                        SceneNode* pFirstSN = hFirstSN.getObject<SceneNode>();
                        Vector3 playerPos = pFirstSN->m_base.getPos();
                        Vector3 forward = pFirstSN->m_base.getN();

                        // 枪口位置：角色前方 0.5 米，高度 1.3 米
                        Vector3 muzzlePos = playerPos + forward * 0.5f + Vector3(0, 1.3f, 0);
						// 射线终点
                        Vector3 endPos = muzzlePos + forward * 20.0f;  // 20米长的射线
                        // 画火光（用黄色/橙色线条模拟）
                        Vector3 color(1.0f, 0.8f, 0.0f);  // 黄橙色

                        Vector3 linepts[] = {
                muzzlePos, color,
                endPos, color
                        };

                        Matrix4x4 identity;
                        identity.loadIdentity();

                        // 画几条射线表示火光
                        /*Matrix4x4 identity;
                        identity.loadIdentity();
                        identity.setPos(muzzlePos);

                        float flashSize = 0.3f;
                        Vector3 linepts[] = {
                            Vector3(0, 0, 0), color,
                            forward * flashSize, color,

                            Vector3(0, 0, 0), color,
                            forward * flashSize * 0.7f + Vector3(0, flashSize * 0.3f, 0), color,

                            Vector3(0, 0, 0), color,
                            forward * flashSize * 0.7f + Vector3(0, -flashSize * 0.3f, 0), color,
                        };*/

                        DebugRenderer::Instance()->createLineMesh(false, identity, &linepts[0].m_x, 6, 0);
                    }
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


        void PlayerController::performShoot()
        {
            PE::Handle hFirstSN = getFirstComponentHandle<SceneNode>();
            if (!hFirstSN.isValid()) return;

            SceneNode* pFirstSN = hFirstSN.getObject<SceneNode>();

            Vector3 rayOrigin = pFirstSN->m_base.getPos() + Vector3(0, 1.5f, 0);

            Vector3 rayDir = pFirstSN->m_base.getN();
            rayDir = -rayDir;
            rayDir.normalize();

            m_showMuzzleFlash = true;
			m_muzzleFlashTimer = 0.1f; // 显示火光0.1秒

            PEINFO("=== Shooting ===\n");
            //PEINFO("Player pos: (%.2f, %.2f, %.2f)\n", rayOrigin.m_x, rayOrigin.m_y, rayOrigin.m_z);
            //PEINFO("Ray dir: (%.2f, %.2f, %.2f)\n", rayDir.m_x, rayDir.m_y, rayDir.m_z);
            //PEINFO("Current rotation: %.2f\n", m_currentRotation);

            CharacterControlContext* pCtx = m_pContext->get<CharacterControlContext>();
            ClientGameObjectManagerAddon* pGOM = (ClientGameObjectManagerAddon*)pCtx->getGameObjectManagerAddon();

            float hitDist;
            SoldierNPC* pEnemy = pGOM->findSoldierNPCByRay(rayOrigin, rayDir, m_shootRange, hitDist);

            if (pEnemy)
            {
                pEnemy->takeDamage(m_shootDamage);
                PEINFO("Hit enemy at distance %.2f!\n", hitDist);
            }
            else
            {
                PEINFO("Missed!\n");
            }
        }

        void PlayerController::do_PRE_RENDER_needsRC(PE::Events::Event* pEvt)
{
    PE::Events::Event_PRE_RENDER_needsRC* pRealEvt = (PE::Events::Event_PRE_RENDER_needsRC*)(pEvt);
    
    if (m_gameState == GAME_STATE_MENU)
    {
        // 获取摄像机位置
        CameraSceneNode* pCam = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
        Matrix4x4 camTransform = pCam->m_worldTransform;
        
        Vector3 camPos = camTransform.getPos();
        Vector3 camForward = camTransform.getN();  // 摄像机朝向
        
        // 文字位置：摄像机前方 3 米
        Vector3 textPos = camPos + camForward * 3.0f;
        
        DebugRenderer::Instance()->createTextMesh(
            "Press ENTER to Start",
            false, false, true, false, 0,
            textPos,
            0.01f,  // 大小
            pRealEvt->m_threadOwnershipMask
        );
        
        DebugRenderer::Instance()->createTextMesh(
            "Press ESC to Exit",
            false, false, true, false, 0,
            textPos + Vector3(0, -0.3f, 0),  // 稍微往下
            0.008f,
            pRealEvt->m_threadOwnershipMask
        );
    }
}
    }
}