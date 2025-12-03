#ifndef _CHARACTER_CONTROL_SOLDIER_NPC_
#define _CHARACTER_CONTROL_SOLDIER_NPC_

#include "PrimeEngine/Events/Component.h"


#include "../Events/Events.h"

namespace CharacterControl{

namespace Components {

struct SoldierNPC : public PE::Components::Component
{
	PE_DECLARE_CLASS(SoldierNPC);

	SoldierNPC(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, Events::Event_CreateSoldierNPC *pEvt);

	virtual void addDefaultComponents();

    // 生命值系统
    void takeDamage(float damage);
    bool isAlive() { return !m_isDead; }
    void getWorldAABB(Vector3& outMin, Vector3& outMax);

    float m_health;
    float m_maxHealth;
    bool m_isDead;

    // AABB
    Vector3 m_aabbMin;
    Vector3 m_aabbMax;
};
}; // namespace Components
}; // namespace CharacterControl
#endif

