#include "particles.h"

using namespace std;
using namespace amt;

namespace Particles{
	Model::Model() : dm{ this } {
	}

	TreeNode Model::GetTreeLeaf(Container<StringData> & ctnr, const StringData & data, function<void(const string&)> invalid) {
		return TreeNode(
			[&data]() { return new Panel(data); },
			[&data, &ctnr, invalid](const string& name) { invalid(name); ctnr.Rename(data, name); },
			[&data, &ctnr]() { ctnr.Remove(move(data)); }
		);
	}

	TreeNode Model::GetTreeNode(function<void(const string&)> invalid) {
		return TreeNode{
			[this, invalid](const string& name) {
				invalid(name);
				auto& ctnr = dm.GetContainer();
				ctnr.Insert(StringData(name, ""));
				return GetTreeLeaf(ctnr, ctnr.GetData(name), invalid);
			}
		};
	}

	function<void(const string&)> Model::GetInvalidator(const string & prefix) {
		return [this, &prefix](const string& name) {
			if (!IsStrPrefix(name, prefix)) {
				throw InterfaceException("Particles for this folder must start with \"" + prefix + "\".");
			}
			if (prefix == prefixSFX) {
				for (const auto& hero : wxGetApp().GetHeroList()) {
					if (IsStrPrefix(name, prefixSFX + hero.name)) {
						throw InterfaceException("Particles for the SFX folder must not start with a hero name.");
					}
				}
			}
		};
	}

	function<void(const string&)> Model::GetSkinInvalidator(const Hero& hero, int skin) {
		return [this, &hero, skin](const string& name) {
			if (!(IsStrPrefix(name, hero.name + Hero::GetSkinId(skin)) || IsStrPrefix(name, prefixSFX + hero.name + Hero::GetSkinId(skin)))) {
				throw InterfaceException("Particles for this folder must start with \"" + hero.name + Hero::GetSkinId(skin) + "\" or \"" + prefixSFX + hero.name + Hero::GetSkinId(skin) + "\".");
			}
		};
	}

	function<void(const string&)> Model::GetHeroInvalidator(const Hero& hero) {
		return [this, &hero](const string& name) {
			if (!(IsStrPrefix(name, hero.name) || IsStrPrefix(name, prefixSFX + hero.name))) {
				throw InterfaceException("Particles for this folder must start with \"" + hero.name + "\" or \"" + prefixSFX + hero.name + "\".");
			}
			for (int i = 0; i < hero.numSkins; i++) {
				if (IsStrPrefix(name, hero.name + Hero::GetSkinId(i)) || IsStrPrefix(name, prefixSFX + hero.name + Hero::GetSkinId(i))) {
					throw InterfaceException("Particles for this folder cannot start with a skin prefix such as \"" + hero.name + Hero::GetSkinId(i) + "\" or \"" + prefixSFX + hero.name + Hero::GetSkinId(i) + "\".");
				}
			}
		};
	}

	function<void(const string&)> Model::GetMiscInvalidator() {
	return [this](const string& name) {
		if (IsStrPrefix(name, prefixArea) || IsStrPrefix(name, prefixDP) || IsStrPrefix(name, prefixM) || IsStrPrefix(name, prefixSFX) || IsStrPrefix(name, prefixTutorial) || IsStrPrefix(name, prefixUI)) {
			throw InterfaceException("Reserved names and prefixes cannot be used for misc particles.");
		}
		for (const auto& hero : wxGetApp().GetHeroList()) {
			if (IsStrPrefix(name, hero.name)) {
				throw InterfaceException("Hero names cannot be used as a prefix for misc particles.");
			}
		}
	};
	}

	TreeMap Model::GetTreeMap() {
		auto& ctnr = dm.GetContainer();
		TreeMap root;
		TreeMap gameRoot;
		TreeMap heroRoot;
		TreeMap particles;
		TreeMap nodeArea{ GetTreeNode(GetInvalidator(prefixArea)) };
		TreeMap nodeDP{ GetTreeNode(GetInvalidator(prefixDP)) };
		TreeMap nodeM{ GetTreeNode(GetInvalidator(prefixM)) };
		TreeMap nodeSFX{ GetTreeNode(GetInvalidator(prefixSFX)) };
		TreeMap nodeTutorial{ GetTreeNode(GetInvalidator(prefixTutorial)) };
		TreeMap nodeUI{ GetTreeNode(GetInvalidator(prefixUI)) };
		TreeMap nodeMisc{ GetTreeNode(GetMiscInvalidator()) };
		for (const auto& hero : wxGetApp().GetHeroList()) {
			TreeMap nodeHero;
			TreeMap nodeParticles;
			nodeParticles.insert("<Default>", TreeMap(GetTreeNode(GetHeroInvalidator(hero))));
			for (int i = 0; i < hero.numSkins; i++) {
				nodeParticles.insert(Hero::GetSkinId(i), TreeMap(GetTreeNode(GetSkinInvalidator(hero, i))));
			}
			nodeHero.insert("Particles", move(nodeParticles));
			heroRoot.insert(hero.name, move(nodeHero));
		}
		for (auto& data : ctnr) {
			bool heroDataFlag{ false };
			for (const auto& hero : wxGetApp().GetHeroList()) {
				if (IsStrPrefix(data.GetName(), hero.name) || IsStrPrefix(data.GetName(), prefixSFX + hero.name)) {
					for (int i = 0; i < hero.numSkins; i++) {
						if (IsStrPrefix(data.GetName(), hero.name + Hero::GetSkinId(i)) || IsStrPrefix(data.GetName(), prefixSFX + hero.name + Hero::GetSkinId(i))) {
							//insert into skin folder
							heroRoot.leaves.at(hero.name).leaves.at("Particles").leaves.at(Hero::GetSkinId(i)).insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr,data,GetSkinInvalidator(hero,i))));
							heroDataFlag = true;
							break;
						}
					}
					if (!heroDataFlag) {
						//insert into default folder
						heroRoot.leaves.at(hero.name).leaves.at("Particles").leaves.at("<Default>").insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr,data,GetHeroInvalidator(hero))));
						heroDataFlag = true;
					}
					break;
				}
			}
			if (heroDataFlag) { //already inserted into hero node
				continue;
			}
			else if (IsStrPrefix(data.GetName(), prefixArea)) { //Deal with game node inserts from here down
				nodeArea.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixArea))));
			}
			else if (IsStrPrefix(data.GetName(), prefixDP)) {
				nodeDP.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixDP))));
			}
			else if (IsStrPrefix(data.GetName(), prefixM)) {
				nodeM.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixM))));
			}
			else if (IsStrPrefix(data.GetName(), prefixSFX)) {
				nodeSFX.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixSFX))));
			}
			else if (IsStrPrefix(data.GetName(), prefixTutorial)) {
				nodeTutorial.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixTutorial))));
			}
			else if (IsStrPrefix(data.GetName(), prefixUI)) {
				nodeUI.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixUI))));
			}
			else {
				nodeMisc.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetMiscInvalidator())));
			}
		}
		particles.insert("Affect Area", move(nodeArea));
		particles.insert("Drop Pod", move(nodeDP));
		particles.insert("Menu", move(nodeM));
		particles.insert("SFX", move(nodeSFX));
		particles.insert("Tutorial", move(nodeTutorial));
		particles.insert("UI", move(nodeUI));
		particles.insert("<Misc>", move(nodeMisc));
		gameRoot.insert("Particles", move(particles));
		root.insert(TreeMap::GameDir, move(gameRoot));
		root.insert(TreeMap::HeroDir, move(heroRoot));
		return root;
	}
}