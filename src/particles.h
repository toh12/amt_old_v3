#pragma once

#include <string>
#include <functional>
#include <set>

#include "amt.h"

namespace Particles {
	class Parser : public amt::IStringParser {
	public:
		Parser() : amt::IStringParser{ std::filesystem::path("Data") / std::filesystem::path("Particles"), amt::ParseMode::DEFAULT_BIND } {}
	};

	class Model : public amt::IModel {
		struct Comparitor {
			bool operator() (const std::string& lhs, const std::string& rhs) const {
				return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
			}
		};
		inline static const std::string prefixArea{ "AFFECTAREA_" };
		inline static const std::string prefixDP{ "DP_" };
		inline static const std::string prefixM{ "M_" };
		inline static const std::string prefixSFX{ "SFX_" };
		inline static const std::string prefixTutorial{ "TUTORIAL_" };
		inline static const std::string prefixUI{ "UI_" };
	public:
		Model();
		virtual std::string GetLoadingMessage() const override { return "Loading Particles..."; }
		virtual amt::TreeMap GetTreeMap() override;
	private:
		amt::DataManager<amt::StringData, Parser> dm;
		amt::TreeNode GetTreeLeaf(amt::Container<amt::StringData>& ctnr, const amt::StringData& data, std::function<void(const std::string&)> invalid = [](const std::string&) {});
		amt::TreeNode GetTreeNode(std::function<void(const std::string&)> invalid = [](const std::string&) {});
		std::function<void(const std::string&)> GetInvalidator(const std::string& prefix);
		std::function<void(const std::string&)> GetSkinInvalidator(const amt::Hero& hero, int skin);
		std::function<void(const std::string&)> GetHeroInvalidator(const amt::Hero& hero);
		std::function<void(const std::string&)> GetMiscInvalidator();
	};

	class Panel : public amt::XmlPanel {
	public:
		Panel(const amt::StringData& data) : amt::XmlPanel{ data } {}
		virtual std::string GetTabName() const override { return "Particle - " + ref.GetName(); }
	};
}