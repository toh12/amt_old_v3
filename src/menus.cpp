#include "menus.h"

using namespace std;
using namespace amt;

namespace Menus {
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
				throw InterfaceException("Particles for this folder must start with \"" + prefix + "\".");
			}
		};
	}

	function<void(const string&)> Model::GetMiscInvalidator() {
		return [this](const string& name) {
			for (size_t i = 0; i < numPrefix; i++) {
				if (IsStrPrefix(name, prefixes[i])) {
					throw InterfaceException("Reserved names and prefixes cannot be used for misc menus.");
				}
			}
		};
	}

	TreeMap Model::GetTreeMap() {
		auto& ctnr = dm.GetContainer();
		TreeMap root;
		TreeMap menus;
		TreeMap nodes[numPrefix];
		for (size_t i = 0; i < numPrefix; i++) {
			nodes[i] = TreeMap{ GetTreeNode(GetInvalidator(prefixes[i])) };
		}
		TreeMap nodeMisc{ GetTreeNode(GetMiscInvalidator()) };
		for (auto& data : ctnr) {
			bool isMisc{ true };
			for (size_t i = 0; i < numPrefix; i++) {
				if (IsStrPrefix(data.GetName(), prefixes[i])) {
					nodes[i].insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetInvalidator(prefixes[i]))));
					isMisc = false;
					break;
				}
			}
			if (isMisc) {
				nodeMisc.insert(data.GetName(), TreeMap(GetTreeLeaf(ctnr, data, GetMiscInvalidator())));
			}
		}
		for (size_t i = 0; i < numPrefix; i++) {
			menus.insert(prefixes[i], move(nodes[i]));
		}
		menus.insert("<Misc>", move(nodeMisc));
		root.insert("Menus", move(menus));
		return root;
	}
}