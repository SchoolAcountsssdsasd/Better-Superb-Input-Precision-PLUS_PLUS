#include <cmath>

#include "SubtickInputs.hpp"

static bool s_firstFrame = true;

namespace subtickinputs {

	/// @brief whether to skip custom logic and use vanilla behavior
	/// @return true if playLayer is null, mod is disabled, first frame after
	/// pause/death/init, player died, platformer mode, or robtop's replay mode thing
	bool useVanilla() {
		// clang-format off
		auto* playLayer = PlayLayer::get();
		bool vanilla = !playLayer
			|| !config::modEnabled
			|| s_firstFrame
			|| playLayer->m_playerDied
			|| playLayer->m_isPlatformer
			|| playLayer->m_useReplay;
		// clang-format on

		if (vanilla) {
			// A split tick never reaches SIPlayerObject::update in these states.
			// Do not let inputs queued for that tick leak into a later tick.
			s_pendingSplitInputs.clear();
		}

		return vanilla;
	}
} // namespace subtickinputs

// copied from cbf
#ifdef GEODE_IS_WINDOWS
	#include <Geode/modify/CCEGLView.hpp>
	#include <winuser.h>
class $modify(CCEGLView) {
	void pollEvents() {
		auto* playLayer = PlayLayer::get();
		CCNode* parent = playLayer ? playLayer->getParent() : nullptr;

		// clang-format off
		if (!GetFocus()
			|| !playLayer
			|| !parent
			|| parent->getChildByType<PauseLayer>(0)
			|| playLayer->getChildByType<EndLevelLayer>(0)
			|| playLayer->m_playerDied)
		{
			s_firstFrame = true;
		}
		// clang-format on

		CCEGLView::pollEvents();
	}
};
#else
	#include <Geode/modify/CCScheduler.hpp>
class $modify(CCScheduler) {
	void update(float dt) {
		auto* playLayer = PlayLayer::get();
		CCNode* parent = playLayer ? playLayer->getParent() : nullptr;

		// clang-format off
		if (!playLayer
			|| !parent
			|| parent->getChildByType<PauseLayer>(0)
			|| playLayer->getChildByType<EndLevelLayer>(0)
			|| playLayer->m_playerDied)
		{
			s_firstFrame = true;
		}
		// clang-format on

		CCScheduler::update(dt);
	}
};
#endif

#include <Geode/modify/GJBaseGameLayer.hpp>
class $modify(GJBaseGameLayer) {
	void update(float dt) {
		if (PlayLayer::get() && PlayLayer::get()->m_playerDied) {
			s_firstFrame = true;
		}

		GJBaseGameLayer::update(dt);

		if (s_firstFrame) {
			s_firstFrame = false;
		}
	}
};
