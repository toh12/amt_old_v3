#include "animation.h"

using namespace std;
using namespace amt;

namespace Animation {
	Model::Model() : dm{ this } {
	}

	TreeNode Model::GetTreeLeaf(Container<StringData>& ctnr, const StringData& data, function<void(const string&)> invalid) {
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

	function<void(const string&)> Model::GetInvalidator(const string& prefix) {
		return [this, &prefix](const string& name) {
			if (!IsStrPrefix(name, prefix)) {
				throw InterfaceException("Animations for this folder must start with \"" + prefix + "\".");
			}
			if (prefix == prefixSFX) {
				for (const auto& hero : wxGetApp().GetHeroList()) {
					if (IsStrPrefix(name, prefixSFX + hero.name)) {
						throw InterfaceException("Animations for the SFX folder must not start with a hero name.");
					}
				}
			}
			if (prefix == prefixCH) {
				for (const auto& hero : wxGetApp().GetHeroList()) {
					if (IsStrPrefix(name, prefixCH + hero.name)) {
						throw InterfaceException("Animations for the character folder must not start with a hero name.");
					}
				}
			}
		};
	}

	function<void(const string&)> Model::GetSkinInvalidator(const string& prefix, const Hero& hero, int skin) {
		return [this, &prefix, &hero, skin](const string& name) {
			if (!(IsStrPrefix(name, prefix + hero.name + Hero::GetSkinId(skin)))) {
				throw InterfaceException("Animations for this folder must start with \"" + prefix + hero.name + Hero::GetSkinId(skin) + "\".");
			}
		};
	}

	function<void(const string&)> Model::GetHeroInvalidator(const string& prefix, const Hero& hero) {
		return [this, &prefix, &hero](const string& name) {
			if (!(IsStrPrefix(name, prefix + hero.name))) {
				throw InterfaceException("Animations for this folder must start with \"" + prefix + hero.name + "\".");
			}
			for (int i = 0; i < hero.numSkins; i++) {
				if (IsStrPrefix(name, prefix + hero.name + Hero::GetSkinId(i))) {
					throw InterfaceException("Animations for this folder cannot start with a skin prefix such as \"" + prefix + hero.name + Hero::GetSkinId(i) + "\".");
				}
			}
		};
	}

	function<void(const string&)> Model::GetMiscInvalidator() {
		return [this](const string& name) {
			if ((name.size() == 1) || IsStrPrefix(name, prefixBB) || IsStrPrefix(name, prefixCH) || IsStrPrefix(name, prefixDP) || IsStrPrefix(name, prefixEffectIcon) || IsStrPrefix(name, prefixEffects) || IsStrPrefix(name, prefixEffectValue) || IsStrPrefix(name, prefixGMT) || IsStrPrefix(name, prefixIcon) || IsStrPrefix(name, prefixM) || IsStrPrefix(name, prefixPickup) || IsStrPrefix(name, prefixRP) || IsStrPrefix(name, prefixSFX) || IsStrPrefix(name, prefixTLTP) || IsStrPrefix(name, prefixTutorial) || IsStrPrefix(name, prefixUI)) {
				throw InterfaceException("Reserved names and prefixes cannot be used for misc behaviours.");
			}
			for (const auto& hero : wxGetApp().GetHeroList()) {
				if (IsStrPrefix(name, hero.name)) {
					throw InterfaceException("Hero names cannot be used as a prefix for misc behaviours.");
				}
			}
		};
	}

	TreeMap Model::GetTreeMap() {
		auto& ctnr = dm.GetContainer();
		TreeMap root;
		TreeMap gameRoot;
		TreeMap heroRoot;
		TreeMap animations;
		TreeMap nodeBB{ GetTreeNode(GetInvalidator(prefixBB)) };
		TreeMap nodeCH{ GetTreeNode(GetInvalidator(prefixCH)) };
		TreeMap nodeDP{ GetTreeNode(GetInvalidator(prefixDP)) };
		TreeMap nodeEffectIcon{ GetTreeNode(GetInvalidator(prefixEffectIcon)) };
		TreeMap nodeEffects{ GetTreeNode(GetInvalidator(prefixEffects)) };
		TreeMap nodeEffectValue{ GetTreeNode(GetInvalidator(prefixEffectValue)) };
		TreeMap nodeGMT{ GetTreeNode(GetInvalidator(prefixGMT)) };
		TreeMap nodeIcon{ GetTreeNode(GetInvalidator(prefixIcon)) };
		TreeMap nodeM{ GetTreeNode(GetInvalidator(prefixM)) };
		TreeMap nodePickup{ GetTreeNode(GetInvalidator(prefixPickup)) };
		TreeMap nodeRP{ GetTreeNode(GetInvalidator(prefixRP)) };
		TreeMap nodeSFX{ GetTreeNode(GetInvalidator(prefixSFX)) };
		TreeMap nodeTLTP{ GetTreeNode(GetInvalidator(prefixTLTP)) };
		TreeMap nodeTutorial{ GetTreeNode(GetInvalidator(prefixTutorial)) };
		TreeMap nodeUI{ GetTreeNode(GetInvalidator(prefixUI)) };
		TreeMap nodeLetters;
		TreeMap nodeMisc{ GetTreeNode(GetMiscInvalidator()) };
		for (const auto& hero : wxGetApp().GetHeroList()) {
			TreeMap nodeHero;
			TreeMap nodeAnimations;
			TreeMap nodeDefault;
			nodeDefault.insert("Character", TreeMap(GetTreeNode(GetHeroInvalidator(prefixCH, hero))));
			nodeDefault.insert("Icon", TreeMap(GetTreeNode(GetHeroInvalidator(prefixIcon, hero))));
			nodeDefault.insert("SFX", TreeMap(GetTreeNode(GetHeroInvalidator(prefixSFX, hero))));
			nodeDefault.insert("<Misc>", TreeMap(GetTreeNode(GetHeroInvalidator(noPrefix, hero))));
			nodeAnimations.insert("<Default>", move(nodeDefault));
			for (int i = 0; i < hero.numSkins; i++) {
				TreeMap nodeSkin;
				nodeSkin.insert("Character", TreeMap(GetTreeNode(GetSkinInvalidator(prefixCH,hero,i))));
				nodeSkin.insert("Icon", TreeMap(GetTreeNode(GetSkinInvalidator(prefixIcon, hero, i))));
				nodeSkin.insert("SFX", TreeMap(GetTreeNode(GetSkinInvalidator(prefixSFX, hero, i))));
				nodeSkin.insert("<Misc>", TreeMap(GetTreeNode(GetSkinInvalidator(noPrefix, hero, i))));
				nodeAnimations.insert(Hero::GetSkinId(i), move(nodeSkin));
			}
			nodeHero.insert("Animations", move(nodeAnimations));
			heroRoot.insert(hero.name, move(nodeHero));
		}
		for (auto& data : ctnr) {
			bool heroDataFlag{ false };
			for (const auto& hero : wxGetApp().GetHeroList()) {
				if (IsStrPrefix(data.GetName(), hero.name) || IsStrPrefix(data.GetName(), prefixSFX + hero.name) || IsStrPrefix(data.GetName(), prefixCH + hero.name) || IsStrPrefix(data.GetName(), prefixIcon + hero.name)) {
					for (int i = 0; i < hero.numSkins; i++) {
						if (IsStrPrefix(data.GetName(), prefixCH + hero.name + Hero::GetSkinId(i))) {
							heroRoot.leaves.at(hero.name).leaves.at("Animations").leaves.at(Hero::GetSkinId(i)).leaves.at("Character").insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetSkinInvalidator(prefixCH, hero, i))));
							heroDataFlag = true;
							break;
						}
						else if (IsStrPrefix(data.GetName(), prefixIcon + hero.name + Hero::GetSkinId(i))) {
							heroRoot.leaves.at(hero.name).leaves.at("Animations").leaves.at(Hero::GetSkinId(i)).leaves.at("Icon").insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetSkinInvalidator(prefixIcon, hero, i))));
							heroDataFlag = true;
							break;
						}
						else if (IsStrPrefix(data.GetName(), prefixSFX + hero.name + Hero::GetSkinId(i))) {
							heroRoot.leaves.at(hero.name).leaves.at("Animations").leaves.at(Hero::GetSkinId(i)).leaves.at("SFX").insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetSkinInvalidator(prefixSFX, hero, i))));
							heroDataFlag = true;
							break;
						}
						else if (IsStrPrefix(data.GetName(), hero.name + Hero::GetSkinId(i))) {
							heroRoot.leaves.at(hero.name).leaves.at("Animations").leaves.at(Hero::GetSkinId(i)).leaves.at("<Misc>").insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetSkinInvalidator(noPrefix, hero, i))));
							heroDataFlag = true;
							break;
						}
					}
					if (!heroDataFlag) { //insert into default folder
						if (IsStrPrefix(data.GetName(), prefixCH)) {
							heroRoot.leaves.at(hero.name).leaves.at("Animations").leaves.at("<Default>").leaves.at("Character").insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetHeroInvalidator(prefixCH, hero))));
						}
						else if (IsStrPrefix(data.GetName(), prefixIcon)) {
							heroRoot.leaves.at(hero.name).leaves.at("Animations").leaves.at("<Default>").leaves.at("Icon").insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetHeroInvalidator(prefixIcon, hero))));
						}
						else if (IsStrPrefix(data.GetName(), prefixSFX)) {
							heroRoot.leaves.at(hero.name).leaves.at("Animations").leaves.at("<Default>").leaves.at("SFX").insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetHeroInvalidator(prefixSFX, hero))));
						}
						else {
							heroRoot.leaves.at(hero.name).leaves.at("Animations").leaves.at("<Default>").leaves.at("<Misc>").insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetHeroInvalidator(noPrefix, hero))));
						}
						heroDataFlag = true;
					}
					break;
				}
			}
			if (heroDataFlag) { //already inserted into hero node
				continue;
			}
			else if (IsStrPrefix(data.GetName(), prefixBB)) { //Deal with game node inserts from here down
				nodeBB.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixBB))));
			}
			else if (IsStrPrefix(data.GetName(), prefixCH)) {
				nodeCH.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixCH))));
			}
			else if (IsStrPrefix(data.GetName(), prefixDP)) {
				nodeDP.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixDP))));
			}
			else if (IsStrPrefix(data.GetName(), prefixEffectIcon)) {
				nodeEffectIcon.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixEffectIcon))));
			}
			else if (IsStrPrefix(data.GetName(), prefixEffects)) {
				nodeEffects.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixEffects))));
			}
			else if (IsStrPrefix(data.GetName(), prefixEffectValue)) {
				nodeEffectValue.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixEffectValue))));
			}
			else if (IsStrPrefix(data.GetName(), prefixGMT)) {
				nodeGMT.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixGMT))));
			}
			else if (IsStrPrefix(data.GetName(), prefixIcon)) {
				nodeIcon.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixIcon))));
			}
			else if (IsStrPrefix(data.GetName(), prefixM)) {
				nodeM.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixM))));
			}
			else if (IsStrPrefix(data.GetName(), prefixPickup)) {
				nodePickup.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixPickup))));
			}
			else if (IsStrPrefix(data.GetName(), prefixRP)) {
				nodeRP.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixRP))));
			}
			else if (IsStrPrefix(data.GetName(), prefixSFX)) {
				nodeSFX.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixSFX))));
			}
			else if (IsStrPrefix(data.GetName(), prefixTLTP)) {
				nodeTLTP.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixTLTP))));
			}
			else if (IsStrPrefix(data.GetName(), prefixTutorial)) {
				nodeTutorial.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixTutorial))));
			}
			else if (IsStrPrefix(data.GetName(), prefixUI)) {
				nodeUI.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixUI))));
			}
			else if (data.GetName().size() == 1) {
				nodeLetters.insert(data.GetName(), TreeMap(TreeNode([&data]() { return new Panel(data); })));
			}
			else {
				nodeMisc.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetMiscInvalidator())));
			}
		}
		animations.insert("Building Block", move(nodeBB));
		animations.insert("Character", move(nodeCH));
		animations.insert("Drop Pod", move(nodeDP));
		animations.insert("Effect Icon", move(nodeEffectIcon));
		animations.insert("Effects", move(nodeEffects));
		animations.insert("Effect Value", move(nodeEffectValue));
		animations.insert("Announcement", move(nodeGMT));
		animations.insert("Icon", move(nodeIcon));
		animations.insert("Menu", move(nodeM));
		animations.insert("Pickup", move(nodePickup));
		animations.insert("Replay", move(nodeRP));
		animations.insert("SFX", move(nodeSFX));
		animations.insert("Tooltip", move(nodeTLTP));
		animations.insert("Tutorial", move(nodeTutorial));
		animations.insert("UI", move(nodeUI));
		animations.insert("Letters", move(nodeLetters));
		animations.insert("<Misc>", move(nodeMisc));
		gameRoot.insert("Animations", move(animations));
		root.insert(TreeMap::GameDir, move(gameRoot));
		root.insert(TreeMap::HeroDir, move(heroRoot));
		return root;
	}
}