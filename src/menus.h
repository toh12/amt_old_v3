#pragma once

#include <string>
#include <functional>
#include <set>

#include "amt.h"

namespace Menus {
	class Parser : public amt::IStringParser {
	public:
		Parser() : amt::IStringParser{ std::filesystem::path("Data") / std::filesystem::path("MenusNewMatchmaking"), amt::ParseMode::DEFAULT_BIND } {}
	};

	class Model : public amt::IModel {
		struct Comparitor {
			bool operator() (const std::string& lhs, const std::string& rhs) const {
				return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
			}
		};
		inline static const std::string prefixAnimation{ "Animation" };
		inline static const std::string prefixButton{ "button" };
		inline static const std::string prefixCS{ "CharacterSelect" };
		inline static const std::string prefixChat{ "chat" };
		inline static const std::string prefixCG{ "Custom" };
		inline static const std::string prefixDropdown{ "dropdown" };
		inline static const std::string prefixEnd{ "end" };
		inline static const std::string prefixIG{ "inGame" };
		inline static const std::string prefixLabel{ "label" };
		inline static const std::string prefixMA{ "menuArmory" };
		inline static const std::string prefixME{ "menuEnd" };
		inline static const std::string prefixMO{ "menuOptions" };
		inline static const std::string prefixMP{ "menuProfile" };
		inline static const std::string prefixModding{ "Modding" };
		inline static const std::string prefixParty{ "Party" };
		inline static const std::string prefixPopup{ "popup" };
		inline static const std::string prefixPV{ "PriceValue" };
		inline static const std::string prefixProfile{ "profile" };
		inline static const std::string prefixReplay{ "Replay" };
		inline static const std::string prefixSB{ "Scoreboard" };
		inline static const std::string prefixTutorial{ "Tutorial" };
		inline static const std::string prefixUnlock{ "Unlock" };
		inline static const size_t numPrefix{ 22 };
		inline static const std::string prefixes[numPrefix]{ 
			prefixAnimation, prefixButton,prefixCS,prefixChat,prefixCG,
			prefixDropdown, prefixEnd,prefixIG, prefixLabel, prefixMA,
			prefixME, prefixMO, prefixMP, prefixModding, prefixParty,
			prefixPopup, prefixPV, prefixProfile, prefixReplay, prefixSB, 
			prefixTutorial,	prefixUnlock
		};
	public:
		Model();
		virtual std::string GetLoadingMessage() const override { return "Loading Menus..."; }
		virtual amt::TreeMap GetTreeMap() override;
	private:
		amt::DataManager<amt::StringData, Parser> dm;
		amt::TreeNode GetTreeLeaf(amt::Container<amt::StringData>& ctnr, const amt::StringData& data, std::function<void(const std::string&)> invalid = [](const std::string&) {});
		amt::TreeNode GetTreeNode(std::function<void(const std::string&)> invalid = [](const std::string&) {});
		std::function<void(const std::string&)> GetInvalidator(const std::string& prefix);
		std::function<void(const std::string&)> GetMiscInvalidator();
	};

	class Panel : public amt::XmlPanel {
	public:
		Panel(const amt::StringData& data) : amt::XmlPanel{ data } {}
		virtual std::string GetTabName() const override { return "Menu - " + ref.GetName(); }
	};
}