#include "interface.h"

using namespace std;

namespace amt {
	void TreeNode::merge(TreeNode&& tn) {
		if (tn.canAddLeaf && canAddLeaf) {
			throw InterfaceException("Cannot merge treemaps, ambigous node leaf addition setting");
		}
		else if (tn.canAddLeaf) {
			leafFactory = move(tn.leafFactory);
			canAddLeaf = tn.canAddLeaf;
		}
		if (tn.canDelete != canDelete) {
			throw InterfaceException("Cannot merge treemaps, ambiguous node deletion setting");
		}
		if ((tn.canRename && canRename) || (tn.canRename != canRename)) {
			throw InterfaceException("Cannot merge treemaps, ambiguous node renaming setting");
		}
		if (tn.canDisplay && canDisplay) {
			throw InterfaceException("Cannot merge treemaps, ambiguous node display setting");
		}
		else if (tn.canDisplay) {
			panelFactory = move(tn.panelFactory);
			canDisplay = tn.canDisplay;
		}
	}

	void TreeMap::merge(TreeMap&& tm) {
		node.merge(move(tm.node));
		for (auto leaf = tm.leaves.begin(); !tm.leaves.empty(); leaf = tm.leaves.begin()) {
			if (leaves.count(leaf->first)) {
				leaves.at(leaf->first).merge(move(leaf->second));
				tm.leaves.erase(leaf);
			}
			else {
				leaves.insert(tm.leaves.extract(leaf->first));
			}
		}
	}

	IDataManager::IDataManager(IModel* handle) : handle{ handle } {
		handle->managers.push_back(this);
	}

	IDataManager::~IDataManager() {
		auto& vect = handle->managers;
		vect.erase(remove(vect.begin(), vect.end(), this), vect.end());
	}

	void IModel::Init() {
		for (auto dm : managers) {
			dm->Init();
		}
	}

	void IModel::Load() {
		for (auto dm : managers) {
			dm->Load();
		}
	}

	void IModel::Save() {
		for (auto dm : managers) {
			dm->Save();
		}
	}

	bool IModel::IsStrEqual(const string& lhs, const string& rhs) const {
		return (_stricmp(lhs.c_str(), rhs.c_str()) == 0);
	}

	bool IModel::IsStrPrefix(const string& str, const string& prefix) const {
		return (_strnicmp(str.c_str(), prefix.c_str(), prefix.size()) == 0);
	}
}