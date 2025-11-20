#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "Player.h"

#include "PlayerControls.h"

// Inter-Engine includes
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Scene/Mesh.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Networking/EventManager.h"
#include "PrimeEngine/Networking/Client/ClientNetworkManager.h"
#include "CharacterControl/Events/Events.h"
#include "PrimeEngine/GameObjectModel/GameObjectManager.h"
#include "PrimeEngine/Events/StandardKeyboardEvents.h"
#include "PrimeEngine/Events/StandardIOSEvents.h"
#include "PrimeEngine/Events/StandardGameEvents.h"
#include "PrimeEngine/Events/EventQueueManager.h"
#include "PrimeEngine/Events/StandardControllerEvents.h"
#include "PrimeEngine/GameObjectModel/DefaultGameControls/DefaultGameControls.h"
#include "CharacterControl/CharacterControlContext.h"
#include "CharacterControl/Tank/ClientTank.h"
#include "CharacterControl/Client/ClientSpaceShipControls.h"

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

// Arkane Control Values
#define Analog_To_Digital_Trigger_Distance 0.5f
static float Debug_Fly_Speed = 8.0f; //Units per second
#define Debug_Rotate_Speed 2.0f //Radians per second
#define Player_Keyboard_Rotate_Speed 20.0f //Radians per second

namespace CharacterControl {
	namespace Components {

		PE_IMPLEMENT_CLASS1(Player, PE::Components::Component);

		Player::Player(PE::GameContext& context, PE::MemoryArena arena, PE::Handle myHandle, float speed,
			Vector3 spawnPos, float networkPingInterval)
			: PE::Components::Component(context, arena, myHandle)
			, m_timeSpeed(speed)
			, m_time(0)
			, m_active(0)
		{
			m_spawnPos = spawnPos;
			m_overriden = false;
		}

		void Player::addDefaultComponents()
		{
			Component::addDefaultComponents();
			
			PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, Player::do_UPDATE);
			
			PE_REGISTER_EVENT_HANDLER(Event_Player_MoveX, Player::do_Player_MoveX);
			PE_REGISTER_EVENT_HANDLER(Event_Player_MoveY, Player::do_Player_MoveY);

		}
		
		void Player::do_Player_MoveX(PE::Events::Event* pEvt)
		{
			Event_Player_MoveX *pRealEvent = (Event_Player_MoveX*)(pEvt);

			PE::Handle hPlayerSN = getFirstComponentHandle<SceneNode>();
			if (!hPlayerSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}

			SceneNode* pFirstSN = hPlayerSN.getObject<SceneNode>();

			pFirstSN->m_base.moveRight(pRealEvent->m_moveX.getX());
		}

		void Player::do_Player_MoveY(PE::Events::Event* pEvt)
		{
			Event_Player_MoveY* pRealEvent = (Event_Player_MoveY*)(pEvt);
			
			PE::Handle hPlayerSN = getFirstComponentHandle<SceneNode>();
			if (!hPlayerSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}

			SceneNode* pFirstSN = hPlayerSN.getObject<SceneNode>();

			pFirstSN->m_base.moveRight(pRealEvent->m_moveY.getY());
		}

		void Player::do_UPDATE(PE::Events::Event* pEvt)
		{
			PE::Events::Event_UPDATE* pRealEvent = (PE::Events::Event_UPDATE*)(pEvt);
			
			if (m_active) 
			{
				m_time += pRealEvent->m_frameTime;
			}

			PE::Handle hPlayerSN = getFirstComponentHandle<SceneNode>();
			if(!hPlayerSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}

			SceneNode* pFirstSN = hPlayerSN.getObject<SceneNode>();

			static float x = 0.0f;
			static float y = 6.0f;
			static float z = -11.0f;

			// note we could have stored the camera reference in this object instead of searching for camera scene node
			if (CameraSceneNode* pCamSN = pFirstSN->getFirstComponent<CameraSceneNode>())
			{
				pCamSN->m_base.setPos(Vector3(x, y, z));
			}

			if (m_overriden)
			{
				pFirstSN->m_base = m_transformOverride;
			}

		}

		void Player::overrideTransform(Matrix4x4& t)
		{
			m_overriden = true;
			m_transformOverride = t;
		}

		void Player::activate()
		{
			m_active = true;


			PE_REGISTER_EVENT_HANDLER(Event_Player_MoveX, Player::do_Player_MoveX);
			PE_REGISTER_EVENT_HANDLER(Event_Player_MoveY, Player::do_Player_MoveY);

			PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
			if (!hFisrtSN.isValid())
			{
				assert(!"wrong setup. must have scene node referenced");
				return;
			}

			//create camera
			PE::Handle hCamera("Camera", sizeof(Camera));
			Camera* pCamera = new(hCamera) Camera(*m_pContext, m_arena, hCamera, hFisrtSN);
			pCamera->addDefaultComponents();
			CameraManager::Instance()->setCamera(CameraManager::VEHICLE, hCamera);

			CameraManager::Instance()->selectActiveCamera(CameraManager::VEHICLE);

			//disable default camera controls

			m_pContext->getDefaultGameControls()->setEnabled(false);
			m_pContext->get<CharacterControlContext>()->getSpaceShipGameControls()->setEnabled(false);
			//enable tank controls

			m_pContext->get<CharacterControlContext>()->getTankGameControls()->setEnabled(true);
		
		}
	};
};