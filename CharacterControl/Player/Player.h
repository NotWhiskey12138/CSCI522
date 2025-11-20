#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Math/Vector3.h"
#include "PrimeEngine/Math/Matrix4x4.h"

namespace PE {
	namespace Events{
		struct EventQueueManager;
	}
}

namespace CharacterControl {
	namespace Components {

		struct Player : public PE::Components::Component
		{
			// component API
			PE_DECLARE_CLASS(Player);

			Player(PE::GameContext& context, PE::MemoryArena arena,PE::Handle myHandle, 
				Vector3 spawnPos); // constructor

			virtual void addDefaultComponents(); // adds default children and event handlers

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
			virtual void do_UPDATE(PE::Events::Event* pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Player_MoveX);
			virtual void do_Player_MoveX(PE::Events::Event* pEvt);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_Player_MoveY);
			virtual void do_Player_MoveY(PE::Events::Event* pEvt);

			void overrideTransform(Matrix4x4& t);
			void activate();
			float m_timeSpeed; // 时间速度（控制路径动画之类）
			float m_time; // 累积时间

			Vector3 m_spawnPos;	// 出生位置
			bool m_active;	// 是否正在由玩家控制
			bool m_overriden;           // transform 是否被外部接管
			Matrix4x4 m_transformOverride; // 如果接管，用这个矩阵
			
			

		};

	}; // namespace Components
}; // namespace CharacterControl

#endif