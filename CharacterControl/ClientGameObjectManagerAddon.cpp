#include "ClientGameObjectManagerAddon.h"

#include "PrimeEngine/PrimeEngineIncludes.h"

#include "Characters/SoldierNPC.h"
#include "WayPoint.h"
#include "Tank/ClientTank.h"
#include "CharacterControl/Client/ClientSpaceShip.h"
#include "CharacterControl/Player/PlayerController.h"

#include "Characters/SoldierNPCAnimationSM.h"
#include "PrimeEngine/Scene/SkeletonInstance.h"
#include "CharacterControl/Events/Events.h"

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;
using namespace CharacterControl::Components;

namespace CharacterControl{
namespace Components
{
PE_IMPLEMENT_CLASS1(ClientGameObjectManagerAddon, Component); // creates a static handle and GteInstance*() methods. still need to create construct

void ClientGameObjectManagerAddon::addDefaultComponents()
{
	GameObjectManagerAddon::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(Event_CreateSoldierNPC, ClientGameObjectManagerAddon::do_CreateSoldierNPC);
	PE_REGISTER_EVENT_HANDLER(Event_CREATE_WAYPOINT, ClientGameObjectManagerAddon::do_CREATE_WAYPOINT);

	// note this component (game obj addon) is added to game object manager after network manager, so network manager will process this event first
	PE_REGISTER_EVENT_HANDLER(PE::Events::Event_SERVER_CLIENT_CONNECTION_ACK, ClientGameObjectManagerAddon::do_SERVER_CLIENT_CONNECTION_ACK);

	PE_REGISTER_EVENT_HANDLER(Event_MoveTank_S_to_C, ClientGameObjectManagerAddon::do_MoveTank);
}

void ClientGameObjectManagerAddon::do_CreateSoldierNPC(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_CreateSoldierNPC>());

	Event_CreateSoldierNPC *pTrueEvent = (Event_CreateSoldierNPC*)(pEvt);

	createSoldierNPC(pTrueEvent);
}

void ClientGameObjectManagerAddon::createSoldierNPC(Vector3 pos, int &threadOwnershipMask)
{
	Event_CreateSoldierNPC evt(threadOwnershipMask);
	evt.m_pos = pos;
	evt.m_u = Vector3(1.0f, 0, 0);
	evt.m_v = Vector3(0, 1.0f, 0);
	evt.m_n = Vector3(0, 0, 1.0f);
	
	StringOps::writeToString( "SoldierTransform.mesha", evt.m_meshFilename, 255);
	StringOps::writeToString( "Soldier", evt.m_package, 255);
	StringOps::writeToString( "mg34.x_mg34main_mesh.mesha", evt.m_gunMeshName, 64);
	StringOps::writeToString( "CharacterControl", evt.m_gunMeshPackage, 64);
	StringOps::writeToString( "", evt.m_patrolWayPoint, 32);
	createSoldierNPC(&evt);
}

void ClientGameObjectManagerAddon::createSoldierNPC(Event_CreateSoldierNPC *pTrueEvent)
{
	PEINFO("CharacterControl: GameObjectManagerAddon: Creating CreateSoldierNPC\n");

	PE::Handle hSoldierNPC("SoldierNPC", sizeof(SoldierNPC));
	SoldierNPC *pSoldierNPC = new(hSoldierNPC) SoldierNPC(*m_pContext, m_arena, hSoldierNPC, pTrueEvent);
	pSoldierNPC->addDefaultComponents();

	// add the soldier as component to the ObjecManagerComponentAddon
	// all objects of this demo live in the ObjecManagerComponentAddon
	addComponent(hSoldierNPC);
}

void ClientGameObjectManagerAddon::do_CREATE_WAYPOINT(PE::Events::Event *pEvt)
{
	PEINFO("GameObjectManagerAddon::do_CREATE_WAYPOINT()\n");

	assert(pEvt->isInstanceOf<Event_CREATE_WAYPOINT>());

	Event_CREATE_WAYPOINT *pTrueEvent = (Event_CREATE_WAYPOINT*)(pEvt);

	PE::Handle hWayPoint("WayPoint", sizeof(WayPoint));
	WayPoint *pWayPoint = new(hWayPoint) WayPoint(*m_pContext, m_arena, hWayPoint, pTrueEvent);
	pWayPoint->addDefaultComponents();

	addComponent(hWayPoint);
}

WayPoint *ClientGameObjectManagerAddon::getWayPoint(const char *name)
{
	PE::Handle *pHC = m_components.getFirstPtr();

	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<WayPoint>())
		{
			WayPoint *pWP = (WayPoint *)(pC);
			if (StringOps::strcmp(pWP->m_name, name) == 0)
			{
				// equal strings, found our waypoint
				return pWP;
			}
		}
	}
	return NULL;
}


void ClientGameObjectManagerAddon::createTank(int index, int &threadOwnershipMask)
{

	//create hierarchy:
	//scene root
	//  scene node // tracks position/orientation
	//    Tank

	//game object manager
	//  TankController
	//    scene node
	
	PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
	MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);

	pMeshInstance->addDefaultComponents();
	pMeshInstance->initFromFile("kingtiger.x_main_mesh.mesha", "Default", threadOwnershipMask);

	// need to create a scene node for this mesh
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	Vector3 spawnPos(-36.0f + 6.0f * index, 0 , 21.0f);
	pSN->m_base.setPos(spawnPos);
	
	pSN->addComponent(hMeshInstance);

	RootSceneNode::Instance()->addComponent(hSN);

	// now add game objects

	PE::Handle hTankController("TankController", sizeof(TankController));
	TankController *pTankController = new(hTankController) TankController(*m_pContext, m_arena, hTankController, 0.05f, spawnPos,  0.05f);
	pTankController->addDefaultComponents();

	addComponent(hTankController);

	// add the same scene node to tank controller
	static int alllowedEventsToPropagate[] = {0}; // we will pass empty array as allowed events to propagate so that when we add
	// scene node to the square controller, the square controller doesnt try to handle scene node's events
	// because scene node handles events through scene graph, and is child of square controller just for referencing purposes
	pTankController->addComponent(hSN, &alllowedEventsToPropagate[0]);
}

void ClientGameObjectManagerAddon::createSpaceShip(int &threadOwnershipMask)
{

	//create hierarchy:
	//scene root
	//  scene node // tracks position/orientation
	//    SpaceShip

	//game object manager
	//  SpaceShipController
	//    scene node

	PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
	MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);

	pMeshInstance->addDefaultComponents();
	pMeshInstance->initFromFile("space_frigate_6.mesha", "FregateTest", threadOwnershipMask);

	// need to create a scene node for this mesh
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	Vector3 spawnPos(0, 0, 0.0f);
	pSN->m_base.setPos(spawnPos);

	pSN->addComponent(hMeshInstance);

	RootSceneNode::Instance()->addComponent(hSN);

	// now add game objects

	PE::Handle hSpaceShip("ClientSpaceShip", sizeof(ClientSpaceShip));
	ClientSpaceShip *pSpaceShip = new(hSpaceShip) ClientSpaceShip(*m_pContext, m_arena, hSpaceShip, 0.05f, spawnPos,  0.05f);
	pSpaceShip->addDefaultComponents();

	addComponent(hSpaceShip);

	// add the same scene node to tank controller
	static int alllowedEventsToPropagate[] = {0}; // we will pass empty array as allowed events to propagate so that when we add
	// scene node to the square controller, the square controller doesnt try to handle scene node's events
	// because scene node handles events through scene graph, and is child of space ship just for referencing purposes
	pSpaceShip->addComponent(hSN, &alllowedEventsToPropagate[0]);

	pSpaceShip->activate();
}


void ClientGameObjectManagerAddon::do_SERVER_CLIENT_CONNECTION_ACK(PE::Events::Event *pEvt)
{
	Event_SERVER_CLIENT_CONNECTION_ACK *pRealEvt = (Event_SERVER_CLIENT_CONNECTION_ACK *)(pEvt);
	PE::Handle *pHC = m_components.getFirstPtr();

	int itc = 0;
	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<TankController>())
		{
			if (itc == pRealEvt->m_clientId) //activate tank controller for local client based on local clients id
			{
				TankController *pTK = (TankController *)(pC);
				pTK->activate();
				break;
			}
			++itc;
		}
	}
}

void ClientGameObjectManagerAddon::do_MoveTank(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_MoveTank_S_to_C>());

	Event_MoveTank_S_to_C *pTrueEvent = (Event_MoveTank_S_to_C*)(pEvt);

	PE::Handle *pHC = m_components.getFirstPtr();

	int itc = 0;
	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<TankController>())
		{
			if (itc == pTrueEvent->m_clientTankId) //activate tank controller for local client based on local clients id
			{
				TankController *pTK = (TankController *)(pC);
				pTK->overrideTransform(pTrueEvent->m_transform);
				break;
			}
			++itc;
		}
	}
}

// 在 ClientGameObjectManagerAddon.cpp 中添加

void ClientGameObjectManagerAddon::createPlayer(int& threadOwnershipMask)
{
	//create hierarchy:
	//scene root
	//  scene node // tracks position/orientation
	//    Player mesh

	//game object manager
	//  PlayerController
	//    scene node

	//// 创建网格实例
	//PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
	//MeshInstance* pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
	//pMeshInstance->addDefaultComponents();
	//pMeshInstance->initFromFile("SoldierTransform.mesha", "Default", threadOwnershipMask);

	// 创建场景节点
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode* pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	Vector3 spawnPos(0, 0, 0.0f);
	pSN->m_base.setPos(spawnPos);
	
	pSN->m_base.setU(Vector3(1, 0, 0));   // 右方向
	pSN->m_base.setV(Vector3(0, 1, 0));   // 上方向
	pSN->m_base.setN(Vector3(0, 0, 1));   // 前方向

	// ============ 创建士兵骨骼动画系统 ============
	{
		// 1. 创建动画状态机
		PE::Handle hSoldierAnimSM("SoldierNPCAnimationSM", sizeof(SoldierNPCAnimationSM));
		SoldierNPCAnimationSM* pSoldierAnimSM = new(hSoldierAnimSM) SoldierNPCAnimationSM(*m_pContext, m_arena, hSoldierAnimSM);
		pSoldierAnimSM->addDefaultComponents();
		pSoldierAnimSM->m_debugAnimIdOffset = 0;

		// 2. 创建骨骼实例
		PE::Handle hSkeletonInstance("SkeletonInstance", sizeof(SkeletonInstance));
		SkeletonInstance* pSkelInst = new(hSkeletonInstance) SkeletonInstance(*m_pContext, m_arena, hSkeletonInstance,
			hSoldierAnimSM);
		pSkelInst->addDefaultComponents();

		// 加载骨骼和动画
		pSkelInst->initFromFiles("soldier_Soldier_Skeleton.skela", "Soldier", threadOwnershipMask);
		pSkelInst->setAnimSet("soldier_Soldier_Skeleton.animseta", "Soldier");

		// 3. 创建网格实例（附加到骨骼上）
		{
			PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
			MeshInstance* pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
			pMeshInstance->addDefaultComponents();

			// ✅ 网格附加到骨骼，不是直接加载
			pMeshInstance->initFromFile("SoldierTransform.mesha", "Soldier", threadOwnershipMask);

			pSkelInst->addComponent(hMeshInstance);
		}

		// 4. 触发行走动画
		/*Events::SoldierNPCAnimSM_Event_WALK evt;
		pSkelInst->handleEvent(&evt);*/

		// 5. 将骨骼添加到场景节点
		pSN->addComponent(hSkeletonInstance);
	}

	//pSN->addComponent(hMeshInstance);

	RootSceneNode::Instance()->addComponent(hSN);

	// 创建玩家控制器
	PE::Handle hPlayerController("PlayerController", sizeof(PlayerController));
	PlayerController* pPlayerController = new(hPlayerController) PlayerController(
		*m_pContext, m_arena, hPlayerController, spawnPos, 0.05f);
	pPlayerController->addDefaultComponents();

	addComponent(hPlayerController);

	// 添加场景节点引用到控制器
	static int allowedEventsToPropagate[] = { 0 };
	pPlayerController->addComponent(hSN, &allowedEventsToPropagate[0]);

	// 激活玩家控制
	pPlayerController->activate();
}

}
}
