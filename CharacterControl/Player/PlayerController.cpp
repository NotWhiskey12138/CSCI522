#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "PlayerController.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "Player.h"

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl {
    namespace Components {

        PE_IMPLEMENT_CLASS1(PlayerController, Component);

        PlayerController::PlayerController(PE::GameContext& context,PE::MemoryArena arena,PE::Handle hMyself)
            : Component(context, arena, hMyself)
        {
        }

        void PlayerController::addDefaultComponents()
        {
            Component::addDefaultComponents();

            // 只注册 UPDATE，先不写实现
            PE_REGISTER_EVENT_HANDLER(Event_UPDATE, PlayerController::do_UPDATE);
        }

        // 先声明一个空的 do_UPDATE（注意签名必须对）
        void PlayerController::do_UPDATE(PE::Events::Event* pEvt)
        {
            PE::Events::Event_UPDATE* pRealEvent = (PE::Events::Event_UPDATE*)(pEvt);
            float dt = pRealEvent->m_frameTime;

            // 1. 找到父对象的 Player
            CharacterControl::Components::Player* pSol =
                getFirstParentByTypePtr<CharacterControl::Components::Player>();

            if (!pSol)
                return; // 安全保护，避免空指针崩溃

            // 2. 从 Player 上拿第一个 SceneNode（就是构造函数里 addComponent 的那个）
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
            }
        }

    } // namespace Components
} // namespace CharacterControl
