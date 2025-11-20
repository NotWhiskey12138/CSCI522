#ifndef _CHARACTER_CONTROL_CONTEXT_H_
#define _CHARACTER_CONTROL_CONTEXT_H_

#include "PrimeEngine/Utils/PEClassDecl.h"

namespace CharacterControl {
	namespace Components{
		struct GameObjectManagerAddon;
		struct TankGameControls;
		struct SpaceShipGameControls;
		struct SoldierGameControls;
	};

	struct CharacterControlContext : PE::PEAllocatableAndDefragmentable
	{
		Components::GameObjectManagerAddon *getGameObjectManagerAddon(){return m_pGameObjectManagerAddon;}
		Components::TankGameControls *getTankGameControls(){return m_pTankGameControls;}
		Components::SpaceShipGameControls *getSpaceShipGameControls(){return m_pSpaceShipGameControls;}
		Components::SoldierGameControls* getSoldierGameControls() { return m_pSoldierGameControls; }
		Components::SoldierGameControls* m_pSoldierGameControls;
		Components::GameObjectManagerAddon *m_pGameObjectManagerAddon;
		Components::TankGameControls *m_pTankGameControls;
		Components::SpaceShipGameControls *m_pSpaceShipGameControls;
	};

}; // namespace CharacterControl

#endif

