#include "behaviour.h"

using namespace std;
using namespace amt;

BehaviourModel::BehaviourModel() : dm{ this } {
}

TreeNode BehaviourModel::GetTreeLeaf(Container<StringData>& ctnr, const StringData& data, function<void(const string&)> invalid) {
	return TreeNode(
		[&data]() { return new BehaviourPanel(data); },
		[&data, &ctnr, invalid](const string& name) { invalid(name); ctnr.Rename(data, name); },
		[&data, &ctnr]() { ctnr.Remove(move(data)); }
	);
}

TreeNode BehaviourModel::GetTreeStub(Container<StringData>& ctnr, const StringData& data) {
	return TreeNode(
		[&data]() { return new BehaviourPanel(data); }
	);
}
TreeNode BehaviourModel::GetTreeNode(function<void(const string&)> invalid) {
	return TreeNode{
		[this, invalid](const string& name) {
			invalid(name);
			auto& ctnr = dm.GetContainer();
			ctnr.Insert(StringData(name, ""));
			return GetTreeLeaf(ctnr, ctnr.GetData(name), invalid);
		}
	};
}

function<void(const string&)> BehaviourModel::GetInvalidator(const string& prefix) {
	return [this,&prefix](const string& name) {
		if (!IsStrPrefix(name, prefix)) {
			throw InterfaceException("Behaviour for this folder must start with \"" + prefix + "\".");
		}
	};
}

function<void(const string&)> BehaviourModel::GetCommunityInvalidator() {
	return [this](const string& name) {
		if (IsStrPrefix(name, prefixAI) || IsStrPrefix(name, prefixAT) || IsStrPrefix(name, prefixRM) || IsStrPrefix(name, prefixTutorial) || wxGetApp().IsHero(name) || (special.count(name) != 0)) {
			throw InterfaceException("Reserved names and prefixes cannot be used for community behaviours.");
		}
	};
}

TreeMap BehaviourModel::GetTreeMap() {
	auto& ctnr = dm.GetContainer();
	TreeMap root;
	TreeMap game;
	TreeMap hero;
	TreeMap behaviour;
	TreeMap ai{ GetTreeNode(GetInvalidator(prefixAI)) };
	TreeMap atMission{ GetTreeNode(GetInvalidator(prefixAT)) };
	TreeMap rmMission{ GetTreeNode(GetInvalidator(prefixRM)) };
	TreeMap tutorial{ GetTreeNode(GetInvalidator(prefixTutorial)) };
	TreeMap misc;
	TreeMap community{ GetTreeNode(GetCommunityInvalidator()) };
	for (auto& data : ctnr) {
		if (wxGetApp().IsHero(data.GetName())) {
			TreeMap node;
			node.insert("Behaviour", TreeMap(GetTreeStub(ctnr,data)));
			hero.insert(data.GetName(), move(node));
		}
		else if (IsStrPrefix(data.GetName(), prefixAI)) {
			ai.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixAI))));
		}
		else if (IsStrPrefix(data.GetName(), prefixAT)) {
			atMission.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixAT))));
		}
		else if (IsStrPrefix(data.GetName(), prefixRM)) {
			rmMission.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixRM))));
		}
		else if (IsStrPrefix(data.GetName(), prefixTutorial)) {
			tutorial.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixTutorial))));
		}
		else if (special.count(data.GetName())) {
			misc.insert(data.GetName(), TreeMap(GetTreeStub(ctnr, data)));
		}
		else {
			community.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetCommunityInvalidator())));
		}
	}
	behaviour.insert("AI", move(ai));
	behaviour.insert("Mission AT", move(atMission));
	behaviour.insert("Mission RM", move(rmMission));
	behaviour.insert("Tutorial", move(tutorial));
	behaviour.insert("Community", move(community));
	behaviour.insert("Special", move(misc));
	game.insert("Behaviour", move(behaviour));
	root.insert(TreeMap::GameDir, move(game));
	root.insert(TreeMap::HeroDir, move(hero));
	return root;
}