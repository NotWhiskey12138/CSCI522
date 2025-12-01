#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/SkeletonInstance.h"
#include "PrimeEngine/Scene/MeshInstance.h"
#include "PrimeEngine/Scene/RootSceneNode.h"

#include "Vampire.h"
#include "VampireAnimationSM.h"
#include "VampireBehaviorSM.h"
#include "VampireMovementSM.h"

using namespace PE;
using namespace PE::Components;
using namespace CharacterControl::Events;

namespace CharacterControl {

namespace Components {

PE_IMPLEMENT_CLASS1(Vampire, Component);

Vampire::Vampire(PE::GameContext& context, PE::MemoryArena arena, PE::Handle hMyself, Events::Event_CreateVampire* pEvt)
	: Component(context, arena, hMyself)
{
	// 需要获取渲染器上下文才能以线程安全的方式执行。
	m_pContext->getGPUScreen()->AcquireRenderContextOwnership(pEvt->m_threadOwnershipMask);

	//拿到SceneNode的handle
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode* pMainSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pMainSN->addDefaultComponents();

	//设置位置和方向
	pMainSN->m_base.setPos(pEvt->m_pos);
	pMainSN->m_base.setU(pEvt->m_u);
	pMainSN->m_base.setV(pEvt->m_v);
	pMainSN->m_base.setN(pEvt->m_n);

	//将SceneNode添加到RootSceneNode中
	RootSceneNode::Instance()->addComponent(hSN);

	//将SceneNode作为Vampire的组件添加，但不处理任何事件。这只是数据驱动的方式来定位Vampire的SceneNode
	{
		static int allowedEvts[] = { 0 };
		addComponent(hSN, &allowedEvts[0]);
	}

	//添加皮肤
	int numSkins = 1; // 8
	for (int iSkin = 0; iSkin < numSkins; ++iSkin) {
		float z = (iSkin / 4) * 1.5f;
		float x = (iSkin % 4) * 1.5f;

		// 旋转场景节点以正确旋转吸血鬼，因为Maya中的吸血鬼面向错误的方向
		PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode* pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
		pSN->addDefaultComponents();

		pSN->m_base.setPos(Vector3(x, 0, z));

		// 旋转吸血鬼以面向正确的方向
		PE::Handle hRotateSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode* pRotateSN = new(hRotateSN) SceneNode(*m_pContext, m_arena, hRotateSN);
		pRotateSN->addDefaultComponents();

		pRotateSN->m_base.turnLeft(3.1415);

		PE::Handle hVampireAnimSM("VampireAnimationSM", sizeof(VampireAnimationSM));
		VampireAnimationSM* pVampireAnimSM = new(hVampireAnimSM) VampireAnimationSM(*m_pContext, m_arena, hVampireAnimSM);
		pVampireAnimSM->addDefaultComponents();

		pVampireAnimSM->m_debugAnimIdOffset = 0; // rand() % 3;

		PE::Handle hSkeletonInstance("SkeletonInstance", sizeof(SkeletonInstance));
		SkeletonInstance *pSkelInst = new(hSkeletonInstance) SkeletonInstance(*m_pContext, m_arena, hSkeletonInstance,
			hVampireAnimSM);
		pSkelInst->addDefaultComponents();

		//初始化骨骼实例
		pSkelInst->initFromFiles("vampire-t-pose_Hips.skela", "M1", pEvt->m_threadOwnershipMask);

		//设置动画集
		pSkelInst->setAnimSet("vampire-sample-action-pack_Hips.animseta", "M1");

		//创建网格实例
		PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
		MeshInstance* pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();

		pMeshInstance->initFromFile(pEvt->m_meshFilename, pEvt->m_package, pEvt->m_threadOwnershipMask);

		pSkelInst->addComponent(hMeshInstance);

		//将皮肤添加到场景节点
		pRotateSN->addComponent(hSkeletonInstance);

		//将旋转场景节点添加到主场景节点
		pMainSN->addComponent(hSN);
	}

	m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(pEvt->m_threadOwnershipMask);

#if 1
	//添加移动状态机到吸血鬼NPC
	PE::Handle hVampireMovementSM("VampireMovementSM", sizeof(VampireMovementSM));
	VampireMovementSM* pVampireMovementSM = new(hVampireMovementSM) VampireMovementSM(*m_pContext, m_arena, hVampireMovementSM);
	pVampireMovementSM->addDefaultComponents();

	//将其添加到吸血鬼NPC
	addComponent(hVampireMovementSM);

	//添加行为状态机到吸血鬼NPC
	PE::Handle hVampireBehaviorSM("VampireBehaviorSM", sizeof(VampireBehaviorSM));
	VampireBehaviorSM* pVampireBehaviorSM = new(hVampireBehaviorSM) VampireBehaviorSM(*m_pContext, m_arena, hVampireBehaviorSM, hVampireMovementSM);
	pVampireBehaviorSM->addDefaultComponents();

	//将其添加到吸血鬼NPC
	addComponent(hVampireBehaviorSM);

	//设置巡逻路径点
	StringOps::writeToString(pEvt->m_patrolWayPoint, pVampireBehaviorSM->m_curPatrolWayPoint, 32);
	pVampireBehaviorSM->m_havePatrolWayPoint = StringOps::length(pVampireBehaviorSM->m_curPatrolWayPoint) > 0;

	//启动吸血鬼
	pVampireBehaviorSM->start();

#endif // 1
};

void Vampire::addDefaultComponents()
{
	Component::addDefaultComponents();

	//注册事件处理程序
}
};//namespace Components
};//namespace CharacterControl