#pragma once

#include <string>
#include <functional>
#include <set>

#include "amt.h"

namespace Animation {
	class Parser : public amt::IStringParser {
	public:
		Parser() : amt::IStringParser{ std::filesystem::path("Data") / std::filesystem::path("AnimationTemplatesNewMatchmaking"), amt::ParseMode::DEFAULT_BIND } {}
	};

	class Model : public amt::IModel {
		struct Comparitor {
			bool operator() (const std::string& lhs, const std::string& rhs) const {
				return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
			}
		};
		inline static const std::string noPrefix{ "" };
		inline static const std::string prefixBB{ "BB_" };
		inline static const std::string prefixCH{ "CH_" };
		inline static const std::string prefixDP{ "DP_" };
		inline static const std::string prefixEffectIcon{ "EFFECTICON_" };
		inline static const std::string prefixEffects{ "EFFECTS_" };
		inline static const std::string prefixEffectValue{ "EFFECTVALUE_" };
		inline static const std::string prefixGMT{ "GMT_" };
		inline static const std::string prefixIcon{ "ICONCHARACTER_" };
		inline static const std::string prefixM{ "M_" };
		inline static const std::string prefixPickup{ "PICKUP_" };
		inline static const std::string prefixRP{ "RP_" };
		inline static const std::string prefixSFX{ "SFX_" };
		inline static const std::string prefixTLTP{ "TLTP_" };
		inline static const std::string prefixTutorial{ "TUTORIAL_" };
		inline static const std::string prefixUI{ "UI_" };
	public:
		Model();
		virtual std::string GetLoadingMessage() const override { return "Loading Animation Templates..."; }
		virtual amt::TreeMap GetTreeMap() override;
	private:
		amt::DataManager<amt::StringData, Parser> dm;
		amt::TreeNode GetTreeLeaf(amt::Container<amt::StringData>& ctnr, const amt::StringData& data, std::function<void(const std::string&)> invalid = [](const std::string&) {});
		amt::TreeNode GetTreeNode(std::function<void(const std::string&)> invalid = [](const std::string&) {});
		std::function<void(const std::string&)> GetInvalidator(const std::string& prefix);
		std::function<void(const std::string&)> GetSkinInvalidator(const std::string& prefix, const amt::Hero& hero, int skin);
		std::function<void(const std::string&)> GetHeroInvalidator(const std::string& prefix, const amt::Hero& hero);
		std::function<void(const std::string&)> GetMiscInvalidator();
	};

	class Panel : public amt::XmlPanel {
	public:
		Panel(const amt::StringData& data) : amt::XmlPanel{ data } {}
		virtual std::string GetTabName() const override { return "Animation - " + ref.GetName(); }
	};
}