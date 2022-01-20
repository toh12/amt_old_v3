#include "localization.h"

using namespace amt;
using namespace std;
using namespace apcl;

namespace Localization {
	unsigned int Data::FindPrefix(const wstring& str) {
		unsigned int i = 0;
		while (i < prefixes.size()) {
			if (prefixes[i].length() <= str.length()) {
				if (_wcsnicmp(prefixes[i].c_str(), str.c_str(), prefixes[i].length()) == 0) {
					break;
				}
			}
			i++;
		}
		return i;
	}

	TreeMap Model::GetTreeMap() {
		TreeMap root;
		TreeMap game;
		TreeMap hero;
		TreeMap localization;
		for (auto& elem : wxGetApp().GetHeroList()) {
			const string& name{ elem.name };
			TreeMap node;
			TreeMap nodeLocal;
			auto& data = dm.GetContainer().GetData(name);
			auto& dataAUTO = dmAUTO.GetContainer().GetData(name);
			node.insert("Localization", TreeMap(TreeNode([&data]() {return new Panel(const_cast<Data&>(data), true, true); })));
			node.insert("LocalizationAUTO", TreeMap(TreeNode([&dataAUTO]() {return new Panel(const_cast<Data&>(dataAUTO), true, true); })));
			nodeLocal.insert("Localization", move(node));
			hero.insert(name, move(nodeLocal));
		}
		auto& data = dm.GetContainer().GetGlobalData();
		auto& dataAUTO = dmAUTO.GetContainer().GetGlobalData();
		auto& dataIDS = dmIDS.GetContainer().GetGlobalData();
		auto& dataPC = dmPC.GetContainer().GetGlobalData();
		localization.insert(data.GetName(), TreeMap(TreeNode([&data]() {return new Panel(data, true); })));
		localization.insert(dataAUTO.GetName(), TreeMap(TreeNode([&dataAUTO]() {return new Panel(dataAUTO, true); })));
		localization.insert(dataIDS.GetName(), TreeMap(TreeNode([&dataIDS]() {return new Panel(dataIDS); })));
		localization.insert(dataPC.GetName(), TreeMap(TreeNode([&dataPC]() {return new Panel(dataPC); })));
		game.insert("Localization", move(localization));
		root.insert(TreeMap::GameDir, move(game));
		root.insert(TreeMap::HeroDir, move(hero));
		return root;
	}

	Container<Data> LParser::Parse(BindableFolder&& data) {
		Container<Data> ctnr;
        for (auto& file : data) {
			if (treeDisplay) {
				ctnr.InsertGlobal(Data{ file.GetFilename(), "Localization - ", ALL_PREFIX });
			}
			else {
				ctnr.InsertGlobal(Data{ file.GetFilename(), "Localization - " });
			}
			if (splitParse) {
				if (treeDisplay) {
					for (auto& hero : wxGetApp().GetHeroList()) {
						ctnr.Insert(Data{ hero.name, filename + " - ", HERO_PREFIX });
					}
				}
				else {
					for (auto& hero : wxGetApp().GetHeroList()) {
						ctnr.Insert(Data{ hero.name, filename + " - " });
					}
				}
			}
			//read the file as UTF16 LE string, 1st wchar is the ignored file header
			wstringstream ss{ wstring{ reinterpret_cast<wchar_t*>(const_cast<unsigned char*>(file.begin())) + 1, reinterpret_cast<wchar_t*>(const_cast<unsigned char*>(file.end())) } }; 
			wstring str;
			while (getline(ss, str)) {
				if (str.length() > Translation::LANG_NAME.size()) { //ignore empty lines
					if (str.rfind(L'\t',0) != 0 && str.rfind(L"//",0) !=0) { //ignore line of tabs or a comment
						//Create new translation with the id
						size_t begin{ 0 };
						size_t end{ str.find(L'\t', begin) };
						if (end == str.npos) throw InterfaceException("Cannot find id terminating tab in: " + file.GetFilename() + file.GetExtension());
						Translation tl{ str.substr(begin, end - begin) };
						//Split up the languages, assume no errors
						for (int i = 0; i < tl.lang.size(); i++) {
							begin = end + 1;
							end = str.find(L'\t', begin);
							tl.lang[i] = str.substr(begin,end-begin);
						}
						if (splitParse) {
							string key;
							string id{ tl.id.begin(), tl.id.end() }; //trivial case for localization IDs, ASCII only
							for (auto& hero : wxGetApp().GetHeroList()) {
								if (search(id.begin(), id.end(), hero.name.begin(), hero.name.end(), [](char ch1, char ch2) {return tolower(ch1) == tolower(ch2); }) != id.end()) {
									key = hero.name;
									break;
								}
							}
							if (!key.empty()) {
								const_cast<Data&>(ctnr.GetData(key)).translations.insert(move(tl));
							}
							else {
								ctnr.GetGlobalData().translations.insert(move(tl));
							}
						}
						else {
							ctnr.GetGlobalData().translations.insert(move(tl));
						}
					}
				}
			}
        }
		return ctnr;
	}

	BindableFolder LParser::Parse(const Container<Data>& data) {
		//Constants to write to the file
		wstring header{ 0xFFFE };
		wstring tab{ 0x0009 };
		wstring crlf{ 0x000D,0x000A };

		//Merge localizations into a map, with a set localizations they reference
		struct Comparitor {
			bool operator() (const Translation* lhs, const Translation* rhs) const {
				return (*lhs) < (*rhs);
			}
		};
		struct IEqual {
			bool operator() (const wstring& lhs, const wstring& rhs) const {
				return _wcsicmp(lhs.c_str(), rhs.c_str()) < 0;
			}
		};
		map<const Translation*, set<wstring, IEqual>, Comparitor> refMap;
		auto GetReferences = [&refMap](const Data& part) {
			for (auto& tl : part.translations) {
				set<wstring, IEqual> refs;
				for (auto& lang : tl.lang) {
					size_t k = 0;
					k = lang.find(L"[L:", k);
					while (k != lang.npos) {
						k += 3;
						refs.insert(lang.substr(k, lang.find(L"]", k) - k));
						k = lang.find(L"[L:", k);
					}
				}
				refMap.insert({ &tl, move(refs) });
			}
		};
		for (auto& part : data) {
			GetReferences(part);
		}
		GetReferences(data.GetGlobalData());

		//Order Localizations, so that dependencies are listed first
		vector<const Translation*> vect;
		while (!refMap.empty()) {
			bool editFlag{ false };
			//First pass, append all independent localizations to the vector
			for (auto it = refMap.begin(); it != refMap.end();) {
				if (it->second.empty()) {
					vect.push_back(it->first);
					refMap.erase(it++);
					editFlag = true;
				}
				else {
					++it;
				}
			}
			//Second pass, recalculate dependancies.
			for (auto& [tl, refs] : refMap) {
				for (auto it = refs.begin(); it != refs.end();) {
					//If a dependancy is no longer in refMap, delete it
					if (find_if(refMap.begin(), refMap.end(), [&it](const pair<const Translation*, set<wstring,IEqual>>& val) { return (_wcsicmp(val.first->id.c_str(), it->c_str()) == 0); }) == refMap.end()) {
						refs.erase(it++);
					}
					else {
						++it;
					}
				}
			}
			//Circular dependancies or format/spelling errors. Write remaining tls.
			if (!editFlag) {
				for (auto& [tl, refs] : refMap) {
					vect.push_back(tl);
				}
				refMap.clear();
			}
		}

		//Calculate filesize
		unsigned int size = header.length() * 2; //UTF16 - LE header
		size += 4; //CRLF
		for (const Translation* tl : vect) {
			size += 2 * (tl->id.length() + 1); //len(ID) + tab
			for (auto& lang : tl->lang) {
				size += 2 * (lang.length() + 1); //len(EN/FR/DE...) + tab
			}
			size += 4; //CRLF
		}
		BindableFolder ctnr;
		BinaryFile bf{ size,data.GetGlobalData().GetName(),".settings" };

		//Write the vect to the binary file
		auto iter = bf.begin();
		auto WriteStr = [&iter, &bf](const wstring& str) {
			if (iter + (2 * str.length()) > bf.end()) {
				throw InterfaceException("Failed to parse localization data: Binary file index out of range.");
			}
			else {
				copy(reinterpret_cast<const unsigned char*>(str.c_str()), reinterpret_cast<const unsigned char*>(str.c_str()) + (2 * str.length()), iter);
				iter += 2 * str.length();
			}
		};
		WriteStr(header);
		WriteStr(crlf);
		for (const Translation* tl : vect) {
			WriteStr(tl->id);
			WriteStr(tab);
			for (auto& lang : tl->lang) {
				WriteStr(lang);
				WriteStr(tab);
			}
			WriteStr(crlf);
		}
		bf.Save(filesystem::path("Enviroment"));
		ctnr.insert(move(bf));
		return ctnr;
	}

	Panel::Panel(Data& data, bool hasPartData, bool hasHeroPart) : IPanel{}, hasPartData{ hasPartData }, hasHeroPart{ hasHeroPart }, ref{ data } {
		InitTreectrl();
		InitGrid();
		InitSizer();
	}

	void Panel::UpdateData() {
		ref.translations.clear();
		for (auto& tl : tlList) {
			ref.translations.insert(*tl);
		}
	}

	void Panel::InitTreectrl() {
		treectrl = new wxTreeCtrl(this, ID_LocalizationTree, wxDefaultPosition, wxSize(280, 200), wxTR_TWIST_BUTTONS | wxTR_NO_LINES | wxTR_HAS_BUTTONS | wxTR_EDIT_LABELS);
		treectrl->Freeze();
		treectrl->SetImageList(new wxImageList(16, 16));
		treectrl->GetImageList()->Add(wxBitmap("Textures/localizationtab_tree_root.png", wxBITMAP_TYPE_PNG));
		treectrl->GetImageList()->Add(wxBitmap("Textures/localizationtab_tree_node.png", wxBITMAP_TYPE_PNG));
		treectrl->GetImageList()->Add(wxBitmap("Textures/localizationtab_tree_node2.png", wxBITMAP_TYPE_PNG));
		treectrl->GetImageList()->Add(wxBitmap("Textures/localizationtab_tree_leaf.png", wxBITMAP_TYPE_PNG));
		auto root = treectrl->AddRoot(ref.GetName(), 0);
		if (ref.prefixes.empty()) {
			for (auto& refTl : ref.translations) {
				Translation* tl = new Translation{ refTl };
				tlList.insert(tl);
				treectrl->AppendItem(root, tl->id, 3, -1, tl);
			}
		}
		else {
			vector<wxTreeItemId> roots;
			wxTreeItemId miscRoot = treectrl->AppendItem(root, Data::misc, 2);
			for (auto& prefix : ref.prefixes) {
				roots.push_back(treectrl->AppendItem(root, prefix, 1));
			}
			for (auto& refTl : ref.translations) {
				Translation* tl = new Translation{ refTl };
				tlList.insert(tl);
				int k = ref.FindPrefix(tl->id);
				if (k < ref.prefixes.size()) {
					treectrl->AppendItem(roots[k], tl->id, 3, -1, tl);
				}
				else {
					treectrl->AppendItem(miscRoot, tl->id, 3, -1, tl);
				}
			}
			for (auto& item : roots) {
				treectrl->SortChildren(item);
			}
			treectrl->SortChildren(miscRoot);
		}
		treectrl->SortChildren(root);
		treectrl->Expand(root);
		treectrl->Bind(wxEVT_TREE_ITEM_ACTIVATED, &Panel::OnTreeClick, this, ID_LocalizationTree);
		treectrl->Bind(wxEVT_TREE_ITEM_MENU, &Panel::OnTreeItemMenu, this, ID_LocalizationTree);
		treectrl->Bind(wxEVT_TREE_BEGIN_LABEL_EDIT, &Panel::OnTreeBeginLabelEdit, this, ID_LocalizationTree);
		treectrl->Bind(wxEVT_TREE_END_LABEL_EDIT, &Panel::OnTreeEndLabelEdit, this, ID_LocalizationTree);
		treectrl->Thaw();
	}

	void Panel::InitGrid() {
		grid = new wxGrid(this, ID_LocalizationGrid, wxDefaultPosition, wxDefaultSize);
		grid->CreateGrid(Translation::LANG_NAME.size(), 1);
		grid->SetColLabelValue(0, "");
		grid->SetColLabelAlignment(wxALIGN_LEFT, wxALIGN_CENTER);
		for (int i = 0; i < Translation::LANG_NAME.size(); i++) {
			grid->SetRowLabelValue(i, Translation::LANG_NAME[i]);
		}
		grid->SetDefaultRenderer(new wxGridCellAutoWrapStringRenderer());
		grid->SetDefaultEditor(new wxGridCellAutoWrapStringEditor());
		for (int i = 0; i < Translation::LANG_NAME.size(); i++) {
			grid->SetRowSize(i, 50);
			grid->DisableRowResize(i);
			grid->SetReadOnly(i, 0, true);
		}
		grid->DisableColResize(0);
		grid->Bind(wxEVT_SIZE, &Panel::OnResize, this, wxID_ANY);
		grid->Bind(wxEVT_GRID_CELL_CHANGING, &Panel::OnCellChanging, this, ID_LocalizationGrid);
		grid->Bind(wxEVT_GRID_CELL_CHANGED, &Panel::OnCellChanged, this, ID_LocalizationGrid);
	}

	void Panel::InitSizer() {
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(treectrl, 0, wxEXPAND);
		sizer->Add(grid, 1, wxEXPAND);
		SetSizerAndFit(sizer);
	}

	void Panel::FormatGrid() {
		int langLabelWidth{ 125 };
		grid->SetRowLabelSize(langLabelWidth);
		grid->SetColSize(0, wxGetApp().GetMainFrame()->GetSize().GetWidth() - 556 - langLabelWidth);
	}

	void Panel::OnResize(wxSizeEvent& event) {
		FormatGrid();
		event.Skip();
	}

	void Panel::OnTreeClick(wxTreeEvent& event) {
		if (treectrl->GetItemData(event.GetItem()) != nullptr) {
			activeTl = static_cast<Translation*>(treectrl->GetItemData(event.GetItem()));
			grid->SetColLabelValue(0, activeTl->id);
			for (int i = 0; i < Translation::LANG_NAME.size(); i++) {
				grid->SetCellValue(i, 0, activeTl->lang[i]);
				grid->SetReadOnly(i, 0, false);
			}
		}
		else if (treectrl->GetRootItem() != event.GetItem()) {
			treectrl->Toggle(event.GetItem());
		}
	}

	void Panel::OnTreeItemMenu(wxTreeEvent& event) {
		treectrl->SelectItem(event.GetItem());
		Translation* tl = static_cast<Translation*>(treectrl->GetItemData(event.GetItem()));
		wxMenu menu;
		if (tl != nullptr) {
			menu.Append(ID_LocalizationTreeMenuRename, "Rename");
			menu.Bind(wxEVT_MENU, &Panel::OnTreeMenuRename, this, ID_LocalizationTreeMenuRename, -1, event.Clone());
			menu.Append(ID_LocalizationTreeMenuDelete, "Delete");
			menu.Bind(wxEVT_MENU, &Panel::OnTreeMenuDelete, this, ID_LocalizationTreeMenuDelete, -1, event.Clone());
		}
		else if(ref.prefixes.empty() || (event.GetItem() != treectrl->GetRootItem())) {
			menu.Append(ID_LocalizationTreeMenuNew, "New");
			menu.Bind(wxEVT_MENU, &Panel::OnTreeMenuNew, this, ID_LocalizationTreeMenuNew, -1, event.Clone());
		}
		menu.Append(ID_LocalizationTreeMenuExpand, "Expand All");
		menu.Bind(wxEVT_MENU, &Panel::OnTreeMenuExpand, this, ID_LocalizationTreeMenuExpand, -1, event.Clone());
		menu.Append(ID_LocalizationTreeMenuCollapse, "Collapse All");
		menu.Bind(wxEVT_MENU, &Panel::OnTreeMenuCollapse, this, ID_LocalizationTreeMenuCollapse, -1, event.Clone());
		PopupMenu(&menu);
	}

	void Panel::OnTreeMenuNew(wxCommandEvent& event) {
		wxTreeItemId item = static_cast<wxTreeEvent*>(event.GetEventUserData())->GetItem();
		Translation* tl = new Translation{};
		tlList.insert(tl);
		auto newItem = treectrl->PrependItem(item, "", 3, -1, tl);
		wxTextCtrl* textctrl = treectrl->EditLabel(newItem);
		wxTextValidator valid{ wxFILTER_EXCLUDE_CHAR_LIST };
		valid.AddCharExcludes(invalidChar);
		textctrl->SetValidator(valid);
	}

	void Panel::OnTreeMenuRename(wxCommandEvent& event) {
		wxTreeItemId item = static_cast<wxTreeEvent*>(event.GetEventUserData())->GetItem();
		wxTextCtrl* textctrl = treectrl->EditLabel(item);
		wxTextValidator valid{ wxFILTER_EXCLUDE_CHAR_LIST };
		valid.AddCharExcludes(invalidChar);
		textctrl->SetValidator(valid);
	}

	void Panel::OnTreeMenuDelete(wxCommandEvent& event) {
		wxTreeItemId item = static_cast<wxTreeEvent*>(event.GetEventUserData())->GetItem();
		Translation* tl = static_cast<Translation*>(treectrl->GetItemData(item));
		if (tl != nullptr) {
			tlList.erase(tl);
			if (tl == activeTl) {
				grid->SetColLabelValue(0, "");
				for (int i = 0; i < Translation::LANG_NAME.size(); i++) {
					grid->SetCellValue(i, 0, "");
					grid->SetReadOnly(i, 0, true);
				}
				activeTl = nullptr;
			}
			treectrl->Delete(item);
			SetEdited();
		}
	}

	void Panel::OnTreeMenuExpand(wxCommandEvent& event) {
		treectrl->ExpandAll();
	}

	void Panel::OnTreeMenuCollapse(wxCommandEvent& event) {
		treectrl->Freeze();
		wxTreeItemIdValue iter;
		wxTreeItemId item = treectrl->GetFirstChild(treectrl->GetRootItem(), iter);
		while (item != treectrl->GetLastChild(treectrl->GetRootItem())) {
			treectrl->CollapseAllChildren(item);
			item = treectrl->GetNextChild(treectrl->GetRootItem(), iter);
		}
		treectrl->CollapseAllChildren(treectrl->GetLastChild(treectrl->GetRootItem()));
		treectrl->Thaw();
	}

	void Panel::OnTreeBeginLabelEdit(wxTreeEvent& event) {
		if (treectrl->GetItemData(event.GetItem()) == nullptr) {
			event.Veto();
		}
	}

	void Panel::OnTreeEndLabelEdit(wxTreeEvent& event) {
		Translation* tl = static_cast<Translation*>(treectrl->GetItemData(event.GetItem()));
		string name = event.GetLabel().ToStdString();
		wstring wname = event.GetLabel().ToStdWstring();
		wstring parent = treectrl->GetItemText(treectrl->GetItemParent(event.GetItem())).ToStdWstring();
		unsigned int index = ref.FindPrefix(wname);
		auto DoRename = [this, &tl, &wname, &event](){
			tl->id = wname;
			treectrl->SetItemText(event.GetItem(), event.GetLabel());
			treectrl->SortChildren(treectrl->GetItemParent(event.GetItem()));
			if (tl == activeTl) {
				grid->SetColLabelValue(0, activeTl->id);
			}
			SetEdited(); 
		};
		auto ReportError = [this, &tl, &event](string str) {
			wxMessageBox(str, "Error : Invalid value", wxOK | wxICON_ERROR);
			event.Veto();
			if (tl->id.empty()) {
				tlList.erase(tl);
				treectrl->Delete(event.GetItem());
			}
		};
		auto NumHeroInName = [this, &name]() {
			int i = 0;
			for (auto& hero : wxGetApp().GetHeroList()) {
				if (search(name.begin(), name.end(), hero.name.begin(), hero.name.end(), [](char ch1, char ch2) {return tolower(ch1) == tolower(ch2); }) != name.end()) {
					i++;
				}
			}
			return i;
		};
		if (name == L"") {
			event.Veto();
			if (tl->id.empty()) {
				tlList.erase(tl);
				treectrl->Delete(event.GetItem());
			}
		}
		else if (find_if(tlList.begin(), tlList.end(), [this, &wname](Translation* elem) { return Translation(wname) == *elem; }) != tlList.end()) {
			ReportError("Could not set localization name: Name already in use!");
		}
		else if (hasPartData && hasHeroPart && (search(name.begin(), name.end(), ref.GetName().begin(), ref.GetName().end(), [](char ch1, char ch2) {return tolower(ch1) == tolower(ch2); }) == name.end())) {
			ReportError("Could not set localization name: Name does not contain hero name!\nPlease ensure the hero dev name is present in names for this file.");
		}
		else if (hasPartData && hasHeroPart && NumHeroInName() > 1) {
			ReportError("Could not set localization name: Name contains another hero name!\nPlease ensure that no additional hero names are present in names for this file.");
		}
		else if (hasPartData && !hasHeroPart && NumHeroInName() > 0) {
			ReportError("Could not set localization name: Name contains hero name!\nPlease ensure no hero dev names are present in names for this file.");
		}
		else if (ref.prefixes.empty()) {
			DoRename();
		}
		else if (parent == Data::misc && index < ref.prefixes.size()) {
			ReportError("Could not set the localization name: Name uses a prefix!\nPlease recreate the localization under the correct prefix.");
		}
		else if (parent == Data::misc) {
			DoRename();
		}
		else if(index >= ref.prefixes.size()) {
			ReportError("Could not set the localization name: Name has an unrecognized prefix!\nPlease ensure the localization's prefix matches its catagory.");
		}
		else if(ref.prefixes[index] != parent) {
			ReportError("Could not set the localization name: Name has a different prefix than before!\nPlease ensure the localization's prefix matches its catagory.");
		}
		else {
			DoRename();
		}
	}

	void Panel::OnCellChanging(wxGridEvent& event) {
		if (event.GetString().Contains("\t") || event.GetString().Contains("\n") || event.GetString().Contains("\r")) {
			wxMessageBox("Localization cannot contain tabs or newline characters!", "Invalid localization");
			event.Veto();
		}
		else {
			event.Skip();
		}
	}

	void Panel::OnCellChanged(wxGridEvent& event) {
		if (activeTl != nullptr) {
			activeTl->lang[event.GetRow()] = grid->GetCellValue(event.GetRow(), 0);
			SetEdited();
		}
	}
}