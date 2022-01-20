#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <filesystem>

#include <apclib.h>

namespace amt {
	template<typename Data> class Container;
	class IPanel;
	class IModel;

	class InterfaceException : public virtual std::runtime_error {
	public:
		InterfaceException() = default;
		InterfaceException(const std::string& msg) : runtime_error{ msg } {}
		virtual const char* what() const throw () { return runtime_error::what(); }
	};

	struct ParseMode {
		static constexpr unsigned int NONE = 0x00;
		static constexpr unsigned int COMPRESS = 0x01;
		static constexpr unsigned int ENCRYPT = 0x02;
		static constexpr unsigned int BIND = 0x04;
		static constexpr unsigned int DEFAULT = COMPRESS | ENCRYPT;
		static constexpr unsigned int DEFAULT_BIND = COMPRESS | ENCRYPT | BIND;
	};

	class TreeNode {
	public:
		TreeNode() {};
		TreeNode(std::function<IPanel* ()>&& panelFactory) : panelFactory{ std::move(panelFactory) }, canDisplay{ true } {}
		TreeNode(std::function<IPanel* ()>&& panelFactory, std::function<void(const std::string&)>&& renamer, std::function<void()>&& deleter) : panelFactory{ std::move(panelFactory) }, renamer{ std::move(renamer) }, deleter{ std::move(deleter) }, canDisplay{ true }, canRename{ true }, canDelete{ true } {}
		TreeNode(std::function<TreeNode(const std::string&)>&& leafFactory) : leafFactory{ std::move(leafFactory) }, canAddLeaf{ true } {}
		TreeNode(std::function<TreeNode(const std::string&)>&& leafFactory, std::function<void(const std::string&)>&& renamer, std::function<void()>&& deleter) : leafFactory{ std::move(leafFactory) }, renamer{ std::move(renamer) }, deleter{ std::move(deleter) }, canRename{ true }, canAddLeaf{ true }, canDelete{ true } {}
		TreeNode(std::function<TreeNode(const std::string&)>&& leafFactory, std::function<IPanel* ()>&& panelFactory) : leafFactory{ std::move(leafFactory) }, panelFactory{ std::move(panelFactory) }, canDisplay{ true }, canAddLeaf{ true } {}
		TreeNode(std::function<TreeNode(const std::string&)>&& leafFactory, std::function<IPanel* ()>&& panelFactory, std::function<void(const std::string&)>&& renamer, std::function<void()>&& deleter) : leafFactory{ std::move(leafFactory) }, panelFactory{ std::move(panelFactory) }, renamer{ std::move(renamer) }, deleter{ std::move(deleter) }, canDisplay{ true }, canRename{ true }, canAddLeaf{ true }, canDelete{ true } {}
		TreeNode CreateLeaf(const std::string& str) const { return leafFactory(str); }
		IPanel* CreatePanel() const { return panelFactory(); }
		void Rename(const std::string& name) const { renamer(name); }
		void Delete() const { deleter(); }
		bool CanDisplay() const { return canDisplay; }
		bool CanRename() const { return canRename; }
		bool CanDelete() const { return canDelete; }
		bool CanAddLeaf() const { return canAddLeaf; }
		void merge(TreeNode&& tn);
		void SetPanelFactory(std::function<IPanel* ()>&& panelFactory) { this->panelFactory = move(panelFactory); canDisplay = true; }
		void SetRenamer(std::function<bool(const std::string&)>&& renamer) { this->renamer = move(renamer); canRename = true; }
		void SetDeleter(std::function<void()>&& deleter) { this->deleter = move(deleter); canDelete = true; }
		void SetLeafFactory(std::function<bool(const std::string&)>&& leafValidator, std::function<TreeNode(const std::string&)>&& leafFactory) { this->leafFactory = move(leafFactory); canAddLeaf = true; }
	private:
		std::function<IPanel* ()> panelFactory{ []()->IPanel* { throw InterfaceException("Cannot create panel: No factory set!"); } };
		std::function<void(const std::string&)> renamer{ [](const std::string&)->void { throw InterfaceException("Cannot rename node: No renamer set!"); } };
		std::function<TreeNode(const std::string&)> leafFactory{ [](const std::string&)->TreeNode { throw InterfaceException("Cannot create node: No factory set!"); } };
		std::function<void()> deleter{ []()->void { throw InterfaceException("Cannot delete node: No deleter set!"); } };
		bool canDisplay{ false };
		bool canRename{ false };
		bool canDelete{ false };
		bool canAddLeaf{ false };
	};

	struct TreeMap {
		inline static const std::string GameDir{ "Game" };
		inline static const std::string HeroDir{ "Hero" };
		inline static const std::string MenuDir{ "Menu" };
		struct Comparitor {
			bool operator() (const std::string& lhs, const std::string& rhs) const {
				return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
			}
		};
		TreeMap() = default;
		TreeMap(TreeNode&& node) : node{ node } {}
		void merge(TreeMap&& tm);
		void insert(const std::string& key, TreeMap&& tm) { leaves.insert_or_assign(key, std::move(tm)); }
		std::map<std::string, TreeMap, Comparitor> leaves;
		TreeNode node;
	};

	class IData {
		template<typename Data> friend class Container;
	public:
		IData() = default;
		IData(const std::string& key) : key{ key } {}
		virtual ~IData() = default;
		friend bool operator==(const IData& lhs, const IData& rhs) { return _stricmp(lhs.key.c_str(), rhs.key.c_str()) == 0; }
		friend bool operator!=(const IData& lhs, const IData& rhs) { return !operator==(lhs, rhs); }
		friend bool operator< (const IData& lhs, const IData& rhs) { return _stricmp(lhs.key.c_str(), rhs.key.c_str()) < 0; }
		friend bool operator> (const IData& lhs, const IData& rhs) { return  operator< (rhs, lhs); }
		friend bool operator<=(const IData& lhs, const IData& rhs) { return !operator> (lhs, rhs); }
		friend bool operator>=(const IData& lhs, const IData& rhs) { return !operator< (lhs, rhs); }
		const std::string& GetName() const { return key; }
	private:
		std::string key;
	};

	template<typename Data>
	class IParser {
	public:
		IParser() = delete;
		IParser(const std::filesystem::path& directory, unsigned int mode = ParseMode::DEFAULT, std::function<bool(const std::filesystem::path&)>&& predicate = [](const std::filesystem::path&) { return true; }) : directory{ directory }, mode{ mode }, predicate{ predicate }  {}
		virtual ~IParser() {}
		const std::filesystem::path& GetDirectory() const { return directory; }
		unsigned int GetMode() const { return mode; }
		bool Predicate(const std::filesystem::path& fp) const { return predicate(fp); }
		virtual apcl::BindableFolder Parse(const Container<Data>& data) = 0;
		virtual Container<Data> Parse(apcl::BindableFolder&& data) = 0;
		virtual void LoadMetadata() {}
	protected:
		inline static const std::filesystem::path DataDir{ "Data" };
		inline static const std::filesystem::path MetaDir{ "Metadata" };
		inline bool IsStrEqual(const std::string& lhs, const std::string& rhs) const;
		inline bool IsStrPrefix(const std::string& str, const std::string& prefix) const;
	private:
		std::filesystem::path directory;
		unsigned int mode;
		std::function<bool(const std::filesystem::path&)> predicate;
	};

	class IDataManager {
	public:
		IDataManager() = delete;
		IDataManager(const IDataManager&) = delete;
		IDataManager(IModel* handle);
		virtual ~IDataManager();
		virtual void Init() = 0;
		virtual void Load() = 0;
		virtual void Save() = 0;
	private:
		IModel* handle;
	};

	class IModel {
		friend IDataManager;
	public:
		IModel() {}
		void Init();
		void Load();
		void Save();
		virtual std::string GetLoadingMessage() const = 0;
		virtual TreeMap GetTreeMap() = 0;
		bool IsStrEqual(const std::string& lhs, const std::string& rhs) const;
		bool IsStrPrefix(const std::string& str, const std::string& prefix) const;
	private:
		std::vector<IDataManager*> managers{};
	};

	template<typename Data>
	bool IParser<Data>::IsStrEqual(const std::string& lhs, const std::string& rhs) const {
		return (_stricmp(lhs.c_str(), rhs.c_str()) == 0);
	}

	template<typename Data>
	bool IParser<Data>::IsStrPrefix(const std::string& str, const std::string& prefix) const {
		return (_strnicmp(str.c_str(), prefix.c_str(), prefix.size()) == 0);
	}
}