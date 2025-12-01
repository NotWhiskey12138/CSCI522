#ifndef _CHARACTER_VAMPIRE_
#define _CHARACTER_VAMPIRE_	

#include "PrimeEngine/Events/Component.h"

#include "CharacterControl/Events/Events.h"

namespace CharacterControl {

namespace Components {

struct Vampire : public PE::Components::Component
{
	PE_DECLARE_CLASS(Vampire);

	Vampire(PE::GameContext& context, PE::MemoryArena arena, PE::Handle hMyself, Events::Event_CreateVampire* pEvt);

	virtual void addDefaultComponents();

};

};//namespace Components
};//namespace CharacterControl


#endif