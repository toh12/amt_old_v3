/*
* Framework for the data storage. Definies the following
* -Container that stores the data
* -DataManager that holds a Container, loading, saving and parsing to it
* 
* The container class is the root of the class design issues in this version.
* Originally a n-ary tree of binary trees (set), it lacked scope and direction.
* The data storage would have been better dealt with as a specific solution for
* each data type, possibly by having Container be a base class.
*/

#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>
#include <array>
#include <exception>
#include <functional>
#include <memory>
#include <algorithm>
#include <filesystem>
#include <numeric>
#include <cstddef>
#include <limits>
#include <fstream>

#include "interface.h"

namespace amt {
	class ContainerException : public FileException, public InterfaceException {
	public:
		ContainerException(const std::string& msg) : FileException{ msg }, InterfaceException{ msg }, runtime_error{ msg } {}
		virtual const char* what() const throw () { return runtime_error::what(); }
	};

	class Hero {
		inline static const std::vector<std::string> skinId{ "001","002","003","004","005","006","007","008","009","010","011","012","013","014","015","016","017","018","019" };
		inline static const std::filesystem::path HeroInfoFile{ "HeroList.csv" };
	public:
		Hero() = default;
		Hero(std::string name, std::string id, int numSkins) : name{ name }, id{ id }, numSkins{ numSkins } {}
		static const std::string& GetSkinId(int id) { return skinId[id]; }
		static std::vector<Hero> LoadHeroList();
		std::string name;
		std::string id;
		int numSkins{ 0 };
	};

	template<typename Data>
	class Container : private std::set<Data> {
	public:
		Container() = default;
		Container(const std::string& bindPrefix) : bindPrefix{ bindPrefix }, canBind{ true } {}
		inline Data& GetGlobalData();
		inline const Data& GetGlobalData() const;
		inline const Data& GetData(const std::string& name) const;
		inline const std::string& GetBindPrefix() const;
		inline void Insert(Data&& data);
		inline void Insert(const Data& data);
		inline void InsertGlobal(Data&& data);
		inline void InsertGlobal(const Data& data);
		bool IsBindable() const { return canBind; }
		void SetBindPrefix(const std::string& prefix);
		void Remove(Data&& data) { std::set<Data>::erase(std::move(data)); }
		void Remove(const Data& data) { std::set<Data>::erase(data); }
		inline void Rename(const Data& data, const std::string& name);
		inline typename std::set<Data>::iterator begin() noexcept { return std::set<Data>::begin(); }
		inline typename std::set<Data>::const_iterator begin() const noexcept { return std::set<Data>::begin(); }
		inline typename std::set<Data>::iterator end() noexcept { return std::set<Data>::end(); }
		inline typename std::set<Data>::const_iterator end() const noexcept { return std::set<Data>::end(); }
	private:
		inline static const std::string ID_GLOBAL{ "<GLOBAL>" };
		std::string bindPrefix;
		bool hasGlobal{ false };
		bool canBind{ false };
		Data globalData;
	};

	template<typename Data, class Parser>
	class DataManager : IDataManager {
		inline static const std::string ENV_FOLDER{ "Enviroment" };
	public:
		inline DataManager(IModel* handle);
		Container<Data>& GetContainer() { return data; }
		const Container<Data>& GetContainer() const { return data; }
	private:
		Container<Data> data;
		std::unique_ptr<IParser<Data>> parser;
		inline virtual void Init() override;
		inline virtual void Load() override;
		inline virtual void Save() override;
		inline apcl::BinaryFile LoadFile(const std::filesystem::path& fp);
		inline apcl::BindableFolder LoadDirectory(const std::filesystem::path& fp);
		inline void SaveDirectory(const std::filesystem::path& fp, apcl::BindableFolder& bf);
	};

	template<typename Data>
	const Data& Container<Data>::GetData(const std::string& name) const {
		if (name == ID_GLOBAL) {
			if (hasGlobal)
				return globalData;
			else
				throw ContainerException("No global data to retieve");
		}
		else {
			Data temp;
			temp.key = name;
			if (std::set<Data>::count(temp))
				return *(std::set<Data>::find(temp));
			else
				throw ContainerException("No data of the given name found");
		}
	}

	template<typename Data>
	Data& Container<Data>::GetGlobalData() {
		if (hasGlobal)
			return globalData;
		else
			throw ContainerException("No global data to return");
	}

	template<typename Data>
	const Data& Container<Data>::GetGlobalData() const {
		if (hasGlobal)
			return globalData;
		else
			throw ContainerException("No global data to return");
	}

	template<typename Data>
	const std::string& Container<Data>::GetBindPrefix() const {
		if (canBind) {
			return bindPrefix;
		}
		else {
			throw ContainerException("No bind prefix is avaliable to return");
		}
	}

	template<typename Data>
	void Container<Data>::Insert(Data&& data) {
		if (data.key == ID_GLOBAL && hasGlobal) {
			throw ContainerException("Could not insert data \"" + data.key + "\", key already in use.");
		}
		else if (data.key == ID_GLOBAL) {
			globalData = data;
			hasGlobal = true;
		}
		else if (std::set<Data>::count(data)) {
			throw ContainerException("Could not insert data \"" + data.key + "\", key already in use.");
		}
		else {
			std::set<Data>::insert(std::move(data));
		}
	}

	template<typename Data>
	void Container<Data>::Insert(const Data& data) {
		if (data.key == ID_GLOBAL && hasGlobal) {
			throw ContainerException("Could not insert data \"" + data.key + "\", key already in use.");
		}
		else if (data.key == ID_GLOBAL) {
			globalData = data;
			hasGlobal = true;
		}
		else if (count(data)) {
			throw ContainerException("Could not insert data \"" + data.key + "\", key already in use.");
		}
		else {
			insert_or_assign(data);
		}
	}

	template<typename Data>
	void Container<Data>::InsertGlobal(Data&& data) {
		if (hasGlobal) {
			throw ContainerException("Could not insert data \"" + data.key + "\", key already in use.");
		}
		else {
			globalData = data;
			hasGlobal = true;
		}
	}

	template<typename Data>
	void Container<Data>::InsertGlobal(const Data& data) {
		if (hasGlobal) {
			throw ContainerException("Could not insert data \"" + data.key + "\", key already in use.");
		}
		else {
			globalData = data;
			hasGlobal = true;
		}
	}

	template<typename Data>
	void Container<Data>::SetBindPrefix(const std::string& prefix) {
		if (canBind) {
			throw ContainerException("Bind prefix is already set and cannot be changed!");
		}
		else {
			bindPrefix = prefix;
			canBind = true;
		}
	}

	template<typename Data>
	void Container<Data>::Rename(const Data& data, const std::string& key) {
		Data temp;
		temp.key = key;
		if (data.key == ID_GLOBAL) {
			throw ContainerException("Cannot rename global data");
		}
		else if (key == ID_GLOBAL) {
			throw ContainerException("Cannot rename to the global key");
		}
		else if (std::set<Data>::count(temp)) {
			throw ContainerException("Cannot rename to an existing key");
		}
		else {
			auto node = std::set<Data>::extract(data);
			node.value().key = key;
			std::set<Data>::insert(std::move(node));
		}
	}

	template<typename Data, typename Parser>
	DataManager<Data,Parser>::DataManager(IModel* handle)
		: IDataManager{ handle }, parser{ std::unique_ptr<IParser<Data>>(new Parser()) } {
		if (!(std::filesystem::exists(ENV_FOLDER / parser->GetDirectory()) && std::filesystem::is_directory(ENV_FOLDER / parser->GetDirectory()))) {
			throw FileException("Could not find directory: " + parser->GetDirectory().string());
		}
	}

	template<typename Data, typename Parser>
	void DataManager<Data, Parser>::Init() {
		parser->LoadMetadata();
	}

	template<typename Data, typename Parser>
	void DataManager<Data, Parser>::Load() {
		parser->LoadMetadata();
		auto bf = LoadDirectory(parser->GetDirectory());
		data = parser->Parse(std::move(bf));
	}

	template<typename Data, typename Parser>
	void DataManager<Data, Parser>::Save() {
		auto bf = parser->Parse(data);
		SaveDirectory(parser->GetDirectory(), bf);
	}

	template<typename Data, typename Parser>
	apcl::BinaryFile DataManager<Data, Parser>::LoadFile(const std::filesystem::path& fp) {
		auto doLoad = [&fp, this](apcl::BinaryFile&& bf) {
			bf.Load(ENV_FOLDER / fp.parent_path());
			if (parser->GetMode() & ParseMode::ENCRYPT) bf.Decrypt(fp.parent_path());
			if (parser->GetMode() & ParseMode::COMPRESS) bf.Decompress();
			return bf;
		};
		try {
			if (fp.has_stem() && fp.has_extension()) {
				return doLoad(apcl::BinaryFile(std::filesystem::file_size(ENV_FOLDER / fp), fp.stem().string(), fp.extension().string()));
			}
			else if (fp.has_stem()) {
				return doLoad(apcl::BinaryFile(std::filesystem::file_size(ENV_FOLDER / fp), fp.stem().string()));
			}
			else if (fp.has_filename()) {
				return doLoad(apcl::BinaryFile(std::filesystem::file_size(ENV_FOLDER / fp), fp.filename().string()));
			}
			else {
				throw FileException("Could not find binary filename: " + fp.string());
			}
		}
		catch (std::filesystem::filesystem_error& e) {
			throw FileException(e.what());
		}
		
	}

	template<typename Data, typename Parser>
	apcl::BindableFolder DataManager<Data, Parser>::LoadDirectory(const std::filesystem::path& fp) {
		apcl::BindableFolder bf;
		for (const auto& iter : std::filesystem::directory_iterator(ENV_FOLDER / fp)) {
			if (parser->Predicate(iter.path()) && std::filesystem::exists(iter.status()) && iter.path().has_filename()) {
				if (std::filesystem::is_regular_file(iter.status())) {
					bf.insert(LoadFile(fp / iter.path().filename()));
				}
			}
		}
		if (parser->GetMode() & ParseMode::BIND) bf.Unbind();
		return bf;
	}

	template<typename Data, typename Parser>
	void DataManager<Data, Parser>::SaveDirectory(const std::filesystem::path& fp, apcl::BindableFolder& bf) {
		if (parser->GetMode() & ParseMode::BIND) bf.Bind();
		for (auto& file : bf) {
			if (parser->GetMode() & ParseMode::COMPRESS) const_cast<apcl::BinaryFile&>(file).Compress();
			if (parser->GetMode() & ParseMode::ENCRYPT) const_cast<apcl::BinaryFile&>(file).Encrypt(fp);
			const_cast<apcl::BinaryFile&>(file).Save(ENV_FOLDER / fp);
		}
	}
}
