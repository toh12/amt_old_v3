#pragma once

#include <string>
#include <functional>
#include <set>

#include "amt.h"

class BehaviourParser : public amt::IStringParser {
public:
	BehaviourParser() : amt::IStringParser{ std::filesystem::path("Data") / std::filesystem::path("Behaviours"), amt::ParseMode::DEFAULT_BIND } {}
};

class BehaviourModel : public amt::IModel {
	struct Comparitor {
		bool operator() (const std::string& lhs, const std::string& rhs) const {
			return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
		}
	};
	inline static const std::string prefixAI{ "AI" };
	inline static const std::string prefixRM{ "RM" };
	inline static const std::string prefixAT{ "AT" };
	inline static const std::string prefixTutorial{ "tutorial" };
	inline static const std::set<std::string, Comparitor> special{ "BlazerAchievements","BotSkill","HeroCode","leaver_retreat","StatusEffects" };
public:
	BehaviourModel();
	virtual std::string GetLoadingMessage() const override { return "Loading Behaviours..."; }
	virtual amt::TreeMap GetTreeMap() override;
private:
	amt::DataManager<amt::StringData, BehaviourParser> dm;
	amt::TreeNode GetTreeLeaf(amt::Container<amt::StringData>& ctnr, const amt::StringData& data, std::function<void(const std::string&)> invalid = [](const std::string&) {});
	amt::TreeNode GetTreeStub(amt::Container<amt::StringData>& ctnr, const amt::StringData& data);
	amt::TreeNode GetTreeNode(std::function<void(const std::string&)> invalid = [](const std::string&) {});
	std::function<void(const std::string&)> GetInvalidator(const std::string& prefix);
	std::function<void(const std::string&)> GetCommunityInvalidator();
};

class BehaviourPanel : public amt::XmlPanel {
public:
	BehaviourPanel(const amt::StringData& data) : amt::XmlPanel{ data } {}
	virtual std::string GetTabName() const override { return "Behaviour - " + ref.GetName(); }
};