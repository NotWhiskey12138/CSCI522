#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

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
		PE_IMPLEMENT_CLASS0(PlayerControls, PE:Components::Component);

		void PlayerControls::addDefaultComponents()
		{
			Component::addDefaultComponents();
			PE_REGISTER_EVENT_HANDLER(Event_UPDATE, PlayerControls::do_UPDATE);
		}

		void PlayerControls::do_UPDATE(PE::Events::Event *pEvt)
		{
			m_moveX = 0;
			m_moveY = 0;

			// Process input events (controller	 buttons, triggers...)
			PE::Handle iqh = PE::Events::EventQueueManager::Instance()->getEventQueueHandle("input");

			// Process input event -> game event conversion
			while (!iqh.getObject<PE::Events::EventQueue>()->empty())
			{
				PE::Events::Event *pInputEvt = iqh.getObject<PE::Events::EventQueue>()->getFront();
				m_frameTime = ((Event_UPDATE*)(pEvt))->m_frameTime;
				// Have DefaultGameControls translate the input event to GameEvents
				handleIOSDebugInputEvents(pInputEvt);
				handleKeyboardDebugInputEvents(pInputEvt);
				handleControllerDebugInputEvents(pInputEvt);
			}

			// Events are destoryed by destroyFront() but this is called every frame just in case
			iqh.getObject<PE::Events::EventQueue>()->destroy();
		}

		void PlayerControls::handleIOSDebugInputEvents(Event* pEvt)
		{
#if APIABSTRACTION_IOS
			m_pQueueManager = PE::Events::EventQueueManager::Instance();
			if (Event_IOS_TOUCH_MOVED::GetClassId() == pEvt->getClassId())
			{
				Event_IOS_TOUCH_MOVED* pRealEvent = (Event_IOS_TOUCH_MOVED*)(pEvt);

				if (pRealEvent->touchesCount > 1)
				{
					PE::Handle h("EVENT", sizeof(Events::Event_Tank_Throttle));
					Events::Event_Tank_Throttle* flyCameraEvt = new(h) Events::Event_Tank_Throttle;

					Vector3 relativeMovement(0.0f, 0.0f, -30.0f * pRealEvent->m_normalized_dy);
					flyCameraEvt->m_relativeMove = relativeMovement * Debug_Fly_Speed * m_frameTime;
					m_pQueueManager->add(h, QT_GENERAL);
				}
				else
				{
					PE::Handle h("EVENT", sizeof(Event_Tank_Turn));
					Event_Tank_Turn* rotateCameraEvt = new(h) Event_Tank_Turn;

					Vector3 relativeRotate(pRealEvent->m_normalized_dx * 10, 0.0f, 0.0f);
					rotateCameraEvt->m_relativeRotate = relativeRotate * Debug_Rotate_Speed * m_frameTime;
					m_pQueueManager->add(h, QT_GENERAL);
				}
			}
#endif
		};

		void PlayerControls::handleKeyboardDebugInputEvents(Event* pEvt)
		{
			m_pQueueManager = PE::Events::EventQueueManager::Instance();

			if (Event_KEY_A_HELD::GetClassId() == pEvt->getClassId()) 
			{
				m_moveX = -1.0f * Debug_Fly_Speed * m_frameTime;
			}
			else if (Event_KEY_S_HELD::GetClassId() == pEvt->getClassId()) 
			{
				m_moveY = -1.0f * Debug_Fly_Speed * m_frameTime;
			}
			else if (Event_KEY_D_HELD::GetClassId() == pEvt->getClassId()) 
			{
				m_moveX = 1.0f * Debug_Fly_Speed * m_frameTime;
			}
			else if (Event_KEY_W_HELD::GetClassId() == pEvt->getClassId()) 
			{
				m_moveY = 1.0f * Debug_Fly_Speed * m_frameTime;
			}
			else 
			{
				Component::handleEvent(pEvt);
			}

		}

		void PlayerControls::handleControllerDebugInputEvents(Event* pEvt)
		{
			m_pQueueManager = PE::Events::EventQueueManager::Instance();
			if (Event_ANALOG_L_THUMB_MOVE::GetClassId() == pEvt->getClassId())
			{
				Event_ANALOG_L_THUMB_MOVE* pRealEvent = (Event_ANALOG_L_THUMB_MOVE*)(pEvt);

				{
					PE::Handle h("EVENT", sizeof(Events::Event_Player_MoveX));
					Events::Event_Player_MoveX* moveXEvt = new(h) Events::Event_Player_MoveX;

					Vector3 relativeMovement(pRealEvent->m_absPosition.getX(), 0.0f, 0.0f);
					moveXEvt->m_moveX = relativeMovement.getX() * Debug_Fly_Speed * m_frameTime;
					m_pQueueManager->add(h, QT_GENERAL);
				}

				{
					PE::Handle h("EVENT", sizeof(Events::Event_Player_MoveY));
					Events::Event_Player_MoveY* moveYEvt = new(h) Events::Event_Player_MoveY;

					Vector3 relativeMovement(0.0f, 0.0f, pRealEvent->m_absPosition.getY());
					moveYEvt->m_moveY = relativeMovement.getZ() * Debug_Fly_Speed * m_frameTime;
					m_pQueueManager->add(h, QT_GENERAL);
				}
			}
		}

	}; // namespace Components
}; // namespace CharacterControl
