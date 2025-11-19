#ifndef _CHARACTER_CONTROL_SOLDIER_NPC_
#define _CHARACTER_CONTROL_SOLDIER_NPC_

#include "PrimeEngine/Events/Component.h"


#include "../Events/Events.h"

namespace CharacterControl{

namespace Components {

struct Player : public PE::Components::Component
{
	PE_DECLARE_CLASS(Player);

	Player(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, Events::Event_CreatePlayer *pEvt);

	virtual void addDefaultComponents();
};
}; // namespace Components
}; // namespace CharacterControl
#endif

