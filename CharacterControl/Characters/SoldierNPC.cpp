#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/SkeletonInstance.h"
#include "PrimeEngine/Scene/MeshInstance.h"
#include "PrimeEngine/Scene/RootSceneNode.h"

#include "SoldierNPC.h"
#include "SoldierNPCAnimationSM.h"
#include "SoldierNPCMovementSM.h"
#include "SoldierNPCBehaviorSM.h"




using namespace PE;
using namespace PE::Components;
using namespace CharacterControl::Events;

namespace CharacterControl{
namespace Components {

PE_IMPLEMENT_CLASS1(SoldierNPC, Component);

SoldierNPC::SoldierNPC(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, Event_CreateSoldierNPC *pEvt) : Component(context, arena, hMyself)
{

	// hierarchy of soldier and replated components and variables (note variables are just variables, they are not passed events to)
	// scene
	// +-components
	//   +-soldier scene node
	//   | +-components
	//   |   +-soldier skin
	//   |     +-components
	//   |       +-soldier animation state machine
	//   |       +-soldier weapon skin scene node
	//   |         +-components
	//   |           +-weapon mesh

	// game objects
	// +-components
	//   +-soldier npc
	//     +-variables
	//     | +-m_hMySN = soldier scene node
	//     | +-m_hMySkin = skin
	//     | +-m_hMyGunSN = soldier weapon skin scene node
	//     | +-m_hMyGunMesh = weapon mesh
	//     +-components
	//       +-soldier scene node (restricted to no events. this is for state machines to be able to locate the scene node)
	//       +-movement state machine
	//       +-behavior state machine

    
	// need to acquire redner context for this code to execute thread-safe
	m_pContext->getGPUScreen()->AcquireRenderContextOwnership(pEvt->m_threadOwnershipMask);
	
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pMainSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pMainSN->addDefaultComponents();

	pMainSN->m_base.setPos(pEvt->m_pos);
	pMainSN->m_base.setU(pEvt->m_u);
	pMainSN->m_base.setV(pEvt->m_v);
	pMainSN->m_base.setN(pEvt->m_n);


	RootSceneNode::Instance()->addComponent(hSN);

	// add the scene node as component of soldier without any handlers. this is just data driven way to locate scnenode for soldier's components
	{
		static int allowedEvts[] = {0};
		addComponent(hSN, &allowedEvts[0]);
	}

	int numskins = 1; // 8
	for (int iSkin = 0; iSkin < numskins; ++iSkin)
	{
		float z = (iSkin / 4) * 1.5f;
		float x = (iSkin % 4) * 1.5f;
		PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
		pSN->addDefaultComponents();

		pSN->m_base.setPos(Vector3(x, 0, z));
		
		// rotation scene node to rotate soldier properly, since soldier from Maya is facing wrong direction
		PE::Handle hRotateSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode *pRotateSN = new(hRotateSN) SceneNode(*m_pContext, m_arena, hRotateSN);
		pRotateSN->addDefaultComponents();

		pSN->addComponent(hRotateSN);

		pRotateSN->m_base.turnLeft(3.1415);

		PE::Handle hSoldierAnimSM("SoldierNPCAnimationSM", sizeof(SoldierNPCAnimationSM));
		SoldierNPCAnimationSM *pSoldierAnimSM = new(hSoldierAnimSM) SoldierNPCAnimationSM(*m_pContext, m_arena, hSoldierAnimSM);
		pSoldierAnimSM->addDefaultComponents();

		pSoldierAnimSM->m_debugAnimIdOffset = 0;// rand() % 3;

		PE::Handle hSkeletonInstance("SkeletonInstance", sizeof(SkeletonInstance));
		SkeletonInstance *pSkelInst = new(hSkeletonInstance) SkeletonInstance(*m_pContext, m_arena, hSkeletonInstance, 
			hSoldierAnimSM);
		pSkelInst->addDefaultComponents();

		pSkelInst->initFromFiles("soldier_Soldier_Skeleton.skela", "Soldier", pEvt->m_threadOwnershipMask);

		pSkelInst->setAnimSet("soldier_Soldier_Skeleton.animseta", "Soldier");

		PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
		MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();
		
		pMeshInstance->initFromFile(pEvt->m_meshFilename, pEvt->m_package, pEvt->m_threadOwnershipMask);
		
		pSkelInst->addComponent(hMeshInstance);

		// add skin to scene node
		pRotateSN->addComponent(hSkeletonInstance);

		#if !APIABSTRACTION_D3D11
		{
			PE::Handle hMyGunMesh = PE::Handle("MeshInstance", sizeof(MeshInstance));
			MeshInstance *pGunMeshInstance = new(hMyGunMesh) MeshInstance(*m_pContext, m_arena, hMyGunMesh);

			pGunMeshInstance->addDefaultComponents();
			pGunMeshInstance->initFromFile(pEvt->m_gunMeshName, pEvt->m_gunMeshPackage, pEvt->m_threadOwnershipMask);

			// create a scene node for gun attached to a joint

			PE::Handle hMyGunSN = PE::Handle("SCENE_NODE", sizeof(JointSceneNode));
			JointSceneNode *pGunSN = new(hMyGunSN) JointSceneNode(*m_pContext, m_arena, hMyGunSN, 38);
			pGunSN->addDefaultComponents();

			// add gun to joint
			pGunSN->addComponent(hMyGunMesh);

			// add gun scene node to the skin
			pSkelInst->addComponent(hMyGunSN);
		}
		#endif
				
		pMainSN->addComponent(hSN);
	}

	m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(pEvt->m_threadOwnershipMask);
	
#if 1
	// add movement state machine to soldier npc
    PE::Handle hSoldierMovementSM("SoldierNPCMovementSM", sizeof(SoldierNPCMovementSM));
	SoldierNPCMovementSM *pSoldierMovementSM = new(hSoldierMovementSM) SoldierNPCMovementSM(*m_pContext, m_arena, hSoldierMovementSM);
	pSoldierMovementSM->addDefaultComponents();

	// add it to soldier NPC
	addComponent(hSoldierMovementSM);

	// add behavior state machine ot soldier npc
    PE::Handle hSoldierBheaviorSM("SoldierNPCBehaviorSM", sizeof(SoldierNPCBehaviorSM));
	SoldierNPCBehaviorSM *pSoldierBehaviorSM = new(hSoldierBheaviorSM) SoldierNPCBehaviorSM(*m_pContext, m_arena, hSoldierBheaviorSM, hSoldierMovementSM);
	pSoldierBehaviorSM->addDefaultComponents();

	// add it to soldier NPC
	addComponent(hSoldierBheaviorSM);

	StringOps::writeToString(pEvt->m_patrolWayPoint, pSoldierBehaviorSM->m_curPatrolWayPoint, 32);
	pSoldierBehaviorSM->m_havePatrolWayPoint = StringOps::length(pSoldierBehaviorSM->m_curPatrolWayPoint) > 0;

	// start the soldier
	pSoldierBehaviorSM->start();
#endif

	// 初始化生命值和AABB
	m_health = 100.0f;
	m_maxHealth = 100.0f;
	m_isDead = false;
	m_aabbMin = Vector3(-0.5f, 0.0f, -0.5f);
	m_aabbMax = Vector3(0.5f, 2.0f, 0.5f);
}

void SoldierNPC::addDefaultComponents()
{
	Component::addDefaultComponents();

	// custom methods of this component
}

void SoldierNPC::takeDamage(float damage)
{
	if (m_isDead) return;

	m_health -= damage;
	PEINFO("SoldierNPC took %.1f damage, health: %.1f\n", damage, m_health);

	if (m_health <= 0)
	{
		m_health = 0;
		m_isDead = true;
		PEINFO("SoldierNPC died!\n");

		// 找到主场景节点
		PE::Components::SceneNode* pMainSN = getFirstComponent<PE::Components::SceneNode>();
		if (pMainSN)
		{

			// 找到子场景节点
			PE::Components::SceneNode* pChildSN = pMainSN->getFirstComponent<PE::Components::SceneNode>();
			if (pChildSN)
			{

				// 找到旋转场景节点
				PE::Components::SceneNode* pRotateSN = pChildSN->getFirstComponent<PE::Components::SceneNode>();
				if (pRotateSN)
				{

					// 找到骨骼实例
					PE::Components::SkeletonInstance* pSkelInst = pRotateSN->getFirstComponent<PE::Components::SkeletonInstance>();
					if (pSkelInst)
					{
						Events::SoldierNPCAnimSM_Event_DIE dieEvt;
						pSkelInst->handleEvent(&dieEvt);
					}
				}
			}
		}
	}
}

void SoldierNPC::getWorldAABB(Vector3& outMin, Vector3& outMax)
{
	PE::Components::SceneNode* pSN = getFirstComponent<PE::Components::SceneNode>();
	if (pSN)
	{
		Vector3 worldPos = pSN->m_base.getPos();
		outMin = worldPos + m_aabbMin;
		outMax = worldPos + m_aabbMax;
	}
	else
	{
		outMin = m_aabbMin;
		outMax = m_aabbMax;
	}
}

}; // namespace Components
}; // namespace CharacterControl
