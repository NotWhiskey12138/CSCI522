#ifndef _Player_CONTROLLER_H_
#define _Player_CONTROLLER_H_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Scene/SceneNode.h"

#include "../Events/Events.h"

namespace CharacterControl {
    namespace Components {

        struct PlayerController : public PE::Components::Component
        {
            PE_DECLARE_CLASS(PlayerController);

            PlayerController(PE::GameContext& context,PE::MemoryArena arena,PE::Handle hMyself);

            virtual void addDefaultComponents();

            PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE)
                virtual void do_UPDATE(PE::Events::Event* pEvt);

        };

    } // namespace Components
} // namespace CharacterControl

#endif