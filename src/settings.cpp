#include "settings.h"

using namespace std;
using namespace std::filesystem;
using namespace amt;
using namespace apcl;

//The settings have been shoehorned from an old version of AMT into the new framework.
//As a result, this file is a complete mess and ideally would be completely rewritten.

namespace Settings {
    class SettingsTreeData : public wxTreeItemData {
    public:
        SettingsTreeData() : wxTreeItemData{}, canRename{ false }, canDelete{ false }, canAddSetting{ false }, canAddProperty{ false }, setting{ nullptr }, settingType{ nullptr }{}
        SettingsTreeData(Catagory* settingType, bool locked = false) : wxTreeItemData{}, canRename{ false }, canDelete{ false }, canAddSetting{ !locked }, canAddProperty{ false }, setting{ nullptr }, settingType{ settingType }{}
        SettingsTreeData(const Catagory* settingType, bool locked = false) : wxTreeItemData{}, canRename{ false }, canDelete{ false }, canAddSetting{ !locked }, canAddProperty{ false }, setting{ nullptr }, settingType{ const_cast<Catagory*>(settingType) }{}
        SettingsTreeData(Catagory* settingType, Setting* setting, bool locked = false) : wxTreeItemData{}, canRename{ (settingType == nullptr) ? false : !locked }, canDelete{ (settingType == nullptr) ? false : !locked }, canAddSetting{ false }, canAddProperty{ setting->IsEditable() }, setting{ setting }, settingType{ settingType }{}
        SettingsTreeData(const Catagory* settingType, const Setting* setting, bool locked = false) : wxTreeItemData{}, canRename{ (settingType == nullptr) ? false : !locked }, canDelete{ (settingType == nullptr) ? false : !locked }, canAddSetting{ false }, canAddProperty{ setting->IsEditable() }, setting{ const_cast<Setting*>(setting) }, settingType{ const_cast<Catagory*>(settingType) }{}
        Setting* GetSetting() const { return setting; }
        Catagory* GetSettingType() const { return settingType; }
        bool HasSetting() const { return setting != nullptr; }
        bool HasRenameMenu() const { return canRename; }
        bool HasDeleteMenu() const { return canDelete; }
        bool HasAddSettingMenu() const { return canAddSetting; }
        bool HasAddPropertyMenu() const { return canAddProperty; }
    private:
        Setting* setting;
        bool canRename;
        bool canDelete;
        bool canAddSetting;
        bool canAddProperty;
        Catagory* settingType;
    };

    SettingDialog::SettingDialog(wxWindow* parent, const string& title, const string& caption, Data* settings, Catagory* settingType) : wxDialog{ parent, wxID_ANY, title, wxDefaultPosition, wxSize(350,150), wxDEFAULT_DIALOG_STYLE, "SettingDialog" }, settings{ settings }, settingType{ settingType }, name{ "" }, textctrl{ new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(300, 20), wxTE_PROCESS_ENTER) } {
        Center();
        wxTextValidator valid{ wxFILTER_EXCLUDE_CHAR_LIST };
        valid.SetCharExcludes(invalidChar);
        textctrl->SetValidator(valid);
        wxButton* buttonOk = new wxButton(this, wxID_OK, "Ok", wxDefaultPosition, wxSize(65, 30));
        Bind(wxEVT_TEXT_ENTER, &SettingDialog::OnTextEnter, this, wxID_ANY);
        Bind(wxEVT_BUTTON, &SettingDialog::OnOk, this, wxID_OK);
        textctrl->Bind(wxEVT_CONTEXT_MENU, [](wxCommandEvent&) {});
        wxBoxSizer* horBotSizer = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer* vertSizer = new wxBoxSizer(wxVERTICAL);
        horBotSizer->AddSpacer(150);
        horBotSizer->Add(buttonOk);
        horBotSizer->AddSpacer(25);
        horBotSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(65, 30)));
        vertSizer->AddSpacer(10);
        vertSizer->Add(new wxStaticText(this, wxID_ANY, "  " + caption), 0, wxALIGN_LEFT);
        vertSizer->AddSpacer(15);
        vertSizer->Add(textctrl, 0, wxALIGN_CENTER);
        vertSizer->AddSpacer(15);
        vertSizer->Add(horBotSizer, 0, wxALIGN_CENTER);
        vertSizer->AddSpacer(3);
        SetSizer(vertSizer);
    }

    void SettingDialog::OnOk(wxCommandEvent& event) {
        name = textctrl->GetValue().ToStdString();
        if (settings->count(Setting(name,{},settingType)) != 0) {
            wxMessageBox("Could not add new setting: Name already in use!", "Error : Invalid value", wxOK | wxICON_ERROR);
        }
        else if (name == "") {
            event.SetId(wxID_CANCEL);
            event.Skip();
        }
        else {
            event.Skip();
        }
    }

    void SettingDialog::OnTextEnter(wxCommandEvent& event) {
        auto newEvent = event.Clone();
        newEvent->SetId(wxID_OK);
        newEvent->SetEventType(wxEVT_BUTTON);
        wxQueueEvent(this, newEvent);
    }
    PropDialog::PropDialog(wxWindow* parent, const string& title, const string& caption, Setting* setting, const string& propName) : wxDialog{ parent, wxID_ANY, title, wxDefaultPosition, wxSize(350,150), wxDEFAULT_DIALOG_STYLE, "UpgradeDialog" }, textctrl{ new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(300, 20), wxTE_PROCESS_ENTER) }, propName{ propName }, setting{ setting }  {
        Center();
        if (setting->catagory != nullptr) { //special cases, assumes only special case is class
            wxIntegerValidator<unsigned int> valid;
            valid.SetMax(99);
            textctrl->SetValidator(valid);
        }
        textctrl->SetValue(propName);
        wxButton* buttonOk = new wxButton(this, wxID_OK, "Ok", wxDefaultPosition, wxSize(65, 30));
        Bind(wxEVT_BUTTON, &PropDialog::OnOk, this, wxID_OK);
        Bind(wxEVT_TEXT_ENTER, &PropDialog::OnTextEnter, this, wxID_ANY);
        textctrl->Bind(wxEVT_CONTEXT_MENU, [](wxCommandEvent&) {});
        wxBoxSizer* horTopSizer = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer* horBotSizer = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer* vertSizer = new wxBoxSizer(wxVERTICAL);
        horTopSizer->Add(textctrl);
        horBotSizer->AddSpacer(150);
        horBotSizer->Add(buttonOk);
        horBotSizer->AddSpacer(25);
        horBotSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(65, 30)));
        vertSizer->AddSpacer(10);
        vertSizer->Add(new wxStaticText(this, wxID_ANY, "  " + caption), 1, wxALIGN_LEFT);
        vertSizer->Add(horTopSizer, 1, wxALIGN_CENTER);
        vertSizer->Add(horBotSizer, 1, wxALIGN_CENTER);
        vertSizer->AddSpacer(3);
        SetSizer(vertSizer);
    }

    void PropDialog::OnOk(wxCommandEvent& event) {
        propName = textctrl->GetValue().ToStdString();
        if (propName == "") {
            event.SetId(wxID_CANCEL);
            event.Skip();
        }
        else if (setting->catagory == nullptr) {
            
            if (find_if(setting->begin(), setting->end(), [this](const Property& p) {return _stricmp(propName.c_str(), p.name.c_str()) == 0; }) != setting->end()) {
                wxMessageBox("Could not add new property: Name already in use!", "Error : Invalid value", wxOK | wxICON_ERROR);
            }
            else {
                event.Skip();
            }
        }
        else { //special cases, assumes only special case is class
            if (propName.length() == 1) {
                propName = "0" + propName;
            }
            propName = { propName[propName.length() - 2], propName[propName.length() - 1] };
            if (find_if(setting->begin(), setting->end(), [this](const Property& p) {return _stricmp(propName.c_str(), p.name.c_str()) == 0; }) != setting->end()) { //Need to change to run through all settings files including All.settings
                wxMessageBox("Could not add new class: Id already in use!", "Error : Invalid value", wxOK | wxICON_ERROR);
            }
            else {
                event.Skip();
            }
        }
    }

    void PropDialog::OnTextEnter(wxCommandEvent& event) {
        auto newEvent = event.Clone();
        newEvent->SetId(wxID_OK);
        newEvent->SetEventType(wxEVT_BUTTON);
        wxQueueEvent(this, newEvent);
    }

    UpgradeDialog::UpgradeDialog(wxWindow* parent, const string& title, const string& caption, Property* prop, const vector<string>& adjVect, const string& adj, const string& name) : wxDialog{ parent, wxID_ANY, title, wxDefaultPosition, wxSize(350,150), wxDEFAULT_DIALOG_STYLE, "UpgradeDialog" }, combobox{ new wxComboBox(this, wxID_ANY, "", wxDefaultPosition, wxSize(40, 25)) }, textctrl{ new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(250, combobox->GetMinHeight()), wxTE_PROCESS_ENTER) }, adj{ adj }, name{ name }, prop{ prop }, defaultAdj{ adj }, defaultName{ name }  {
        Center();
        wxArrayString arr;
        for (string str : adjVect) {
            arr.Add(str);
        }
        combobox->Set(arr);
        combobox->SetEditable(false);
        combobox->SetValue(adj);
        wxTextValidator valid{ wxFILTER_EXCLUDE_CHAR_LIST };
        valid.SetCharExcludes(invalidChar);
        textctrl->SetValidator(valid);
        textctrl->SetValue(name);
        wxButton* buttonOk = new wxButton(this, wxID_OK, "Ok", wxDefaultPosition, wxSize(65, 30));
        Bind(wxEVT_BUTTON, &UpgradeDialog::OnOk, this, wxID_OK);
        Bind(wxEVT_TEXT_ENTER, &UpgradeDialog::OnTextEnter, this, wxID_ANY);
        textctrl->Bind(wxEVT_CONTEXT_MENU, [](wxCommandEvent&) {});
        wxBoxSizer* horTopSizer = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer* horBotSizer = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer* vertSizer = new wxBoxSizer(wxVERTICAL);
        horTopSizer->Add(combobox);
        horTopSizer->AddSpacer(10);
        horTopSizer->Add(textctrl);
        horBotSizer->AddSpacer(150);
        horBotSizer->Add(buttonOk);
        horBotSizer->AddSpacer(25);
        horBotSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(65, 30)));
        vertSizer->AddSpacer(10);
        vertSizer->Add(new wxStaticText(this, wxID_ANY, "  " + caption), 1, wxALIGN_LEFT);
        vertSizer->Add(horTopSizer, 1, wxALIGN_CENTER);
        vertSizer->Add(horBotSizer, 1, wxALIGN_CENTER);
        vertSizer->AddSpacer(3);
        SetSizer(vertSizer);
    }

    void UpgradeDialog::OnOk(wxCommandEvent& event) {
        if (textctrl->GetValue() == "") {
            wxMessageBox("Upgrade name cannot be empty!", "Error : Invalid value", wxOK | wxICON_EXCLAMATION);
        }
        else if (combobox->GetValue() == "") {
            wxMessageBox("Please select how the upgrade is applied!", "Error : Invalid value", wxOK | wxICON_EXCLAMATION);
        }
        else {
            adj = combobox->GetValue();
            name = textctrl->GetValue();
            if ((_stricmp(adj.c_str(), defaultAdj.c_str()) == 0) && (_stricmp(name.c_str(), defaultAdj.c_str()) == 0)) {
                event.Skip();
            }
            else if (prop->count(Upgrade(adj, name,"")) != 0) {
                wxMessageBox("Upgrade name cannot match an existing upgrade for this property!", "Error : Invalid value", wxOK | wxICON_EXCLAMATION);
            }
            else {
                event.Skip();
            }
        }
    }

    void UpgradeDialog::OnTextEnter(wxCommandEvent& event) {
        auto newEvent = event.Clone();
        newEvent->SetId(wxID_OK);
        newEvent->SetEventType(wxEVT_BUTTON);
        wxQueueEvent(this, newEvent);
    }

    string Property::DefaultOf(Type type) {
        switch (type) {
        case Type::Bool:
            return "false";
        case Type::Int:
            return "0";
        case Type::Dbl:
            return "0";
        case Type::Pair:
            return "0 0";
        case Type::Color:
            return "0 0 0";
        case Type::Str:
            return "";
        default:
            return "";
        }
    }

    TreeMap Model::GetTreeMap() {
        auto& ctnr = dm.GetContainer();
        TreeMap root;
        TreeMap game;
        TreeMap hero;
        TreeMap settings;
        for (auto& data : ctnr) {
            if (wxGetApp().IsHero(data.GetName())) {
                TreeMap node;
                node.insert("Settings", TreeMap(TreeNode([&data]() { return new Panel(const_cast<Data&>(data)); })));
                hero.insert(data.GetName(), move(node));
            }
            else {
                settings.insert(data.GetName(), TreeMap(TreeNode([&data]() {return new Panel(const_cast<Data&>(data)); })));
            }
        }
        settings.insert("Global", TreeMap(TreeNode([&ctnr]() {return new Panel(ctnr.GetGlobalData()); })));
        game.insert("Settings", move(settings));
        root.insert(TreeMap::GameDir, move(game));
        root.insert(TreeMap::HeroDir, move(hero));
        return root;
    }

    Parser::Parser() : IParser<Data>{ path("Data") / path("SettingsNewMatchmaking"), ParseMode::DEFAULT,
        [](const path& fp) {
            if (fp.has_stem()) {
                if (fp.stem().string().length() > HeroFilePrefix.length())
                    return (wxGetApp().IsHero(fp.stem().string().substr(HeroFilePrefix.length())) || fp.stem().string() == SpecialFileName);
                else
                    return fp.stem().string() == SpecialFileName;
            }
            else
                return false;
        } } {
    }

    Metadata Parser::LoadMetadataFile(path&& fp) {
        auto getPropType = [this](const string& str) {
            if (IsStrEqual(str, "Bool")) {
                return Property::Type::Bool;
            }
            else if (IsStrEqual(str, "Int")) {
                return Property::Type::Int;
            }
            else if (IsStrEqual(str, "Dbl")) {
                return Property::Type::Dbl;
            }
            else if (IsStrEqual(str, "Pair")) {
                return Property::Type::Pair;
            }
            else if (IsStrEqual(str, "Color")) {
                return Property::Type::Color;
            }
            else {
                return Property::Type::Str;
            }
        };
        //load the parse list
        ifstream parseListFile{ (MetaDir / MetaSubDir / fp).string() + MetaExtension };
        if (!parseListFile) {
            throw FileException("Error when trying to open the parse list: " + fp.string() + MetaExtension);
        }
        Metadata retVal;
        string filename;
        string tempVal;
        vector<path> fileList;
        while (getline(parseListFile, filename)) {
            fileList.push_back(filename);
        }
        parseListFile.close();

        //load the parse files
        for (auto& file : fileList) {
            Catagory data;
            ifstream parseFile{ (MetaDir / MetaSubDir / MetaTypeDir / file).string() + MetaExtension };
            if (!parseFile) throw FileException("Error when trying to open the parse file: " + file.string() + MetaExtension);
            //load the header
            if (!getline(parseFile, data.name, ',')) throw FileException("Error at header1, parse file is corrupted: " + file.string() + MetaExtension);
            if (!getline(parseFile, tempVal, ',')) throw FileException("Error at header2, parse file is corrupted: " + file.string() + MetaExtension);
            data.locked = (tempVal == "TRUE");
            if (!getline(parseFile, tempVal, ',')) throw FileException("Error at header3, parse file is corrupted: " + file.string() + MetaExtension);
            data.reversed = (tempVal == "TRUE");
            if (!getline(parseFile, tempVal)) throw FileException("Error at header4, parse file is corrupted: " + file.string() + MetaExtension);
            data.special = (tempVal == "TRUE");
            //load the properties
            while (getline(parseFile, tempVal, ',')) {
                Property prop;
                prop.name = tempVal;
                if (!getline(parseFile, tempVal, ',')) throw FileException("Error at body1, parse file is corrupted: " + file.string() + MetaExtension);
                prop.type = getPropType(tempVal);
                if (!getline(parseFile, tempVal, ',')) throw FileException("Error at body2, parse file is corrupted: " + file.string() + MetaExtension);
                if (!getline(parseFile, prop.defaultVal)) throw FileException("Error at body3, parse file is corrupted: " + file.string() + MetaExtension);
                if (tempVal != "TRUE") {
                    prop.defaultVal = prop.DefaultOf(prop.type);
                }
                prop.value = prop.defaultVal;
                data.defaultValue.push_back(prop);
            }
            parseFile.close();
            retVal.push_back(move(data));
        }
        return retVal;
    }

    void Parser::LoadMetadata() {
        SpecialMd = LoadMetadataFile(path(SpecialFileName));
        HeroMd = LoadMetadataFile(path("Hero"));
        GlobalMd = LoadMetadataFile(path(Global));
        GlobalMd.SetGlobal();
    }

    Container<Data> Parser::Parse(BindableFolder&& data) {
        Container<Data> ctnr;
        vector<Setting> globalSettings;
        for (Catagory& elem : GlobalMd) {
            globalSettings.push_back(Setting("", elem.defaultValue, &elem));
        }
        for (auto& elem : data) {
            if (elem.GetFilename() == SpecialFileName) {
                ctnr.Insert(ParseFile(elem.GetFilename(), const_cast<BinaryFile&>(elem), SpecialMd, globalSettings));
            }
            else {
                ctnr.Insert(ParseFile(elem.GetFilename().substr(HeroFilePrefix.length()), const_cast<BinaryFile&>(elem), HeroMd, globalSettings));
            }
        }
        ctnr.InsertGlobal(Data{ Global, &GlobalMd });
        for (Setting& elem : globalSettings) {
            ctnr.GetGlobalData().insert(elem);
        }
        return ctnr;
    }

    BindableFolder Parser::Parse(const Container<Data>& data) {
        BindableFolder ctnr;
        for (auto& val : data) {
            if (wxGetApp().IsHero(val.GetName())) {
                ctnr.insert(ParseData(HeroFilePrefix + val.GetName(), val, data.GetGlobalData()));
            }
            else {
                ctnr.insert(ParseData(val.GetName(), val, data.GetGlobalData()));
            }
        }
        return ctnr;
    }

    //Adapted from old code, TODO: complete rewrite
    Data Parser::ParseFile(string filename, BinaryFile& bf, Metadata& md, vector<Setting>& globalSettings) {
        struct SettingLine {
            string name;
            string prop;
            string adj;
            string upg;
            string val;
        };
        //Check if a character is a newline
        auto isNewlineChar = [](const char& ch) {
            return (ch == '\n') || (ch == '\r');
        };
        //Check if a character is an adjustment
        auto isAdjChar = [this](const char& ch) {
            for (char adj : ADJ_CHAR) {
                if (adj == ch)
                    return true;
            }
            return false;
        };

        //parse binary file into strings
        auto it = bf.begin();
        vector<SettingLine> file;
        while (it < bf.end()) {
            SettingLine line;
            while (*it != SPLIT_USCORE[0] && *it != SPLIT_EQ[0] && !isAdjChar(*it) && it < bf.end()) {
                line.name.push_back(*it);
                it++;
            }
            if (*it == SPLIT_USCORE[0]) {
                it += SPLIT_USCORE.size();
            }
            while (*it != SPLIT_EQ[0] && !isAdjChar(*it) && it < bf.end()) {
                line.prop.push_back(*it);
                it++;
            }
            if (isAdjChar(*it)) {
                while (isAdjChar(*it) && it < bf.end()) {
                    line.adj.push_back(*it);
                    it++;
                }
                while (*it != SPLIT_EQ[0] && it < bf.end()) {
                    line.upg.push_back(*it);
                    it++;
                }
            }
            it += SPLIT_EQ.size();
            while (!isNewlineChar(*it) && it < bf.end()) {
                line.val.push_back(*it);
                it++;
            }
            while (isNewlineChar(*it)) {
                it++;
            }
            file.push_back(move(line));
        }

        //variable initialization
        int i = 0;
        int size = file.size();
        Setting typelessSettings;
        Data typedSettings{ filename, &md };

        //lambda functions used in parse loop
        auto getType = [&md](const string& id, const string& prop) {
            if (prop.empty()) return -1;
            for (int i = 0; i < md.size(); i++) {
                if (_strnicmp(id.c_str(), md[i].name.c_str(), md[i].name.size()) == 0) {
                    if (md[i].defaultValue.size() > 0 && !md[i].reversed) {
                        if (_stricmp(prop.c_str(), md[i].defaultValue[0].name.c_str()) == 0) {
                            return i;
                        }
                    }
                    else {
                        return i;
                    }
                }
            }
            return -1;
        };
        auto getTypeList = [&md](const string& id) {
            vector<Catagory*> types;
            for (int j = 0; j < md.size(); j++) {
                if ((_strnicmp(id.c_str(), md[j].name.c_str(), md[j].name.size()) == 0)) {
                    types.push_back(&md[j]);
                }
            }
            return types;
        };
        auto isPoorlyTyped = [&md, &file, &i](int type) {
            if (file[i].prop.empty() || (type + 1) >= md.size() || md[type].defaultValue.size() == 0) {
                return false;
            }
            else {
                return (_strnicmp(file[i].name.c_str(), md[type + 1].name.c_str(), md[type + 1].name.size()) == 0) && (_stricmp(md[type].defaultValue[0].name.c_str(), file[i].prop.c_str()) == 0);
            }
        };
        auto getName = [&md](const string& str, int type) {
            return str.substr(md[type].name.length(), str.length());
        };
        auto addUpg = [file, size, &i](Property& prop) {
            while (!file[i].adj.empty()) {
                prop.insert(Upgrade(file[i].adj, file[i].upg, file[i].val));
                if (++i >= size) break;
            }
        };
        auto addUpgGlobal = [this, &file, &globalSettings, &md, size, &i](Property& prop, int type = -1, int index = -1) {
            int globalType = -1;
            while (!file[i].adj.empty()) {
                if (file[i].adj.length() == 2 && globalType == -1) { //Skip doing global type lookup when possible
                    for (int j = 0; j < GlobalMd.size(); j++) {
                        if (GlobalMd[j] == md[type]) {
                            globalType = j;
                            break;
                        }
                    }
                }
                if (file[i].adj.length() == 2 && globalType != -1) { //Already done global lookup, avoids repeating
                    globalSettings[globalType][index].insert(Upgrade(file[i].adj, file[i].upg, file[i].val));
                }
                else { //local upgrade
                    prop.insert(Upgrade(file[i].adj, file[i].upg, file[i].val));
                }
                if (++i >= size) break;
            }
        };


        //parse loop
        while (i < size) {
            string& id = file[i].name;
            int type = getType(id, file[i].prop);
            //Setting is typeless, lump into generic pool
            if ((type == -1) && (&md == &SpecialMd)) {
                Property prop(Property::Type::Str, id, file[i].val);
                if (++i >= size) {
                    typelessSettings.push_back(move(prop));
                    break;
                }
                addUpg(prop);
                typelessSettings.push_back(move(prop));
            }
            //Setting is typeless when there shouldn't be any
            else if (type == -1) {
                throw FileException("Parse error: Unexpected typeless setting \"" + file[i].name + "\" found in \"" + bf.GetFilename() + "\".");
            }
            //Setting cannot be uniquely identified by type and name alone, requires expensive property matching to filter it
            else if (isPoorlyTyped(type)) {
                auto typeList = getTypeList(id);
                vector<Setting> settings;
                vector<int> propertyIndex;
                for (int j = 0; j < typeList.size(); j++) {
                    settings.push_back(Setting(id.substr(typeList[j]->name.length()), typeList[j]->defaultValue, typeList[j]));
                    propertyIndex.push_back(0);
                }
                while (_strnicmp(file[i].name.c_str(), md[type].name.c_str(), md[type].name.size()) == 0) {
                    for (int j = 0; j < typeList.size(); j++) {
                        if (propertyIndex[j] < typeList[j]->defaultValue.size()) {
                            if (_stricmp(settings[j][propertyIndex[j]].name.c_str(), typeList[j]->defaultValue[propertyIndex[j]].name.c_str()) == 0) {
                                settings[j][propertyIndex[j]].value = file[i].val;
                                if (++i >= size) break;
                                addUpg(settings[j][propertyIndex[j]]);
                                if (i >= size) break;
                                propertyIndex[j]++;
                                break;
                            }
                        }
                    }
                    if (i >= size) break;
                }
                for (const Setting& setting : settings) {
                    typedSettings.insert(setting);
                }

            }
            //Normal parse
            else {
                string name = getName(id, type);
                //Type has a unique no name setting.
                if (file[i].name.length() == md[type].name.length()) {
                    //The nameless setting is a list
                    if (md[type].special) {
                        Setting setting{ name, {}, &md[type] };
                        while ((_stricmp(id.c_str(), file[i].name.c_str()) == 0) && (file[i].name.length() == md[type].name.length())) {
                            setting.push_back(Property(Property::Type::Str, file[i].prop, file[i].val));
                            if (++i >= size) break;
                            addUpg(setting.back());
                            if (i >= size) break;
                        }
                        typedSettings.insert(move(setting));
                    }
                    //Normal parse, cant reverse a nameless setting
                    else {
                        Setting setting{ name, md[type].defaultValue, &md[type] };
                        for (Property& prop : setting) {
                            prop.value = file[i].val;
                            if (++i >= size) break;
                            addUpg(prop);
                            if (i >= size) break;
                        }
                        typedSettings.insert(move(setting));
                    }
                }
                //Property is the name and the name is the property.
                else if (md[type].reversed) {
                    vector<Setting> reversedSettings;
                    int j = 0;
                    while (_stricmp(file[i].name.c_str(), file[i + j].name.c_str()) == 0) { //Alphabetical ordering breaks grouping, need to know all settings of this type
                        reversedSettings.push_back(Setting(file[i + j].prop, md[type].defaultValue, &md[type]));
                        j++;
                        if (i + j >= file.size()) break;
                    }
                    for (int k = 0; k < md[type].defaultValue.size(); k++) {
                        for (int l = 0; l < reversedSettings.size(); l++) {
                            reversedSettings[l][k].value = file[i].val;
                            if (++i >= size) break;
                            addUpg(reversedSettings[l][k]);
                            if (i >= size) break;
                        }
                        if (i >= size) break;
                        if (getType(file[i].name, file[i].prop) == -1) { //Alphabetical ordering means nameless settings can get mixed in
                            if (&md == &SpecialMd) {
                                typelessSettings.push_back(Property(Property::Type::Str, file[i].name, file[i].val));
                                if (++i >= size) break;
                                addUpg(typelessSettings.back());
                                if (i >= size) break;
                            }
                            else {
                                throw FileException("Parse error: Unexpected typeless setting \"" + file[i].name + "\" found in \"" + bf.GetFilename() + "\".");
                            }
                        }
                    }
                    for (const Setting& setting : reversedSettings) {
                        typedSettings.insert(setting);
                    }
                }
                //Normal parse
                else {
                    Setting setting{ name, md[type].defaultValue, &md[type] };
                    int j = 0;
                    for (Property& prop : setting) {
                        prop.value = file[i].val;
                        if (++i >= size) break;
                        addUpgGlobal(prop, type, j++);
                        if (i >= size) break;
                    }
                    typedSettings.insert(move(setting));
                }
            }
        }
        if (&md == &SpecialMd) {
            typedSettings.insert(move(typelessSettings));
        }
        return typedSettings;
    }

    BinaryFile Parser::ParseData(string filename, const Data& data, const Data& globalData) {
        struct SettingLine {
            string name;
            string prop;
            string adj;
            string upg;
            string val;
        };
        //Variable initialization
        vector<SettingLine> file;
        int i = 0;
        const Setting& typelessSettings = (data.begin()->catagory == nullptr) ? *data.begin() : Setting{};

        //Lambda functions for parse loop
        auto addTypeless = [&i, &typelessSettings, &file](const string& id) {
            if (i < typelessSettings.size()) {
                while (_stricmp(typelessSettings[i].name.c_str(), id.c_str()) < 0) {
                    file.push_back({ typelessSettings[i].name,"","","",typelessSettings[i].value });
                    for (const Upgrade& upg : typelessSettings[i]) {
                        file.push_back({ typelessSettings[i].name, "",upg.adj,upg.name,upg.value });
                    }
                    if (++i >= typelessSettings.size()) break;
                }
            }
        };

        //parse loop to construct returned SettingsFile
        for (const Setting& setting : data) {
            if (setting.catagory == nullptr) continue;
            string settingType = setting.catagory->name;
            string settingName = setting.name;
            if (settingName.empty()) { //add nameless settings: can't have globals or be reversed
                addTypeless(settingType);
                for (const Property& prop : setting) {
                    file.push_back({ settingType,prop.name,"","",prop.value });
                    for (const Upgrade& upg : prop) {
                        file.push_back({ settingType,prop.name,upg.adj,upg.name,upg.value });
                    }
                }
            }
            else if (setting.catagory->reversed) { //add reversed settings
                for (const Property& prop : setting) {
                    addTypeless(settingType + prop.name);
                    file.push_back({ settingType + prop.name, settingName,"","",prop.value });
                    for (const Upgrade& upg : prop) {
                        file.push_back({ settingType + prop.name, settingName,upg.adj,upg.name,upg.value });
                    }
                }
            }
            else { //Normal parse
                addTypeless(settingType + settingName);
                Setting* globalSetting = nullptr;
                for (const Setting& gs : globalData) {
                    if (*gs.catagory == *setting.catagory) {
                        globalSetting = const_cast<Setting*>(&gs);
                        break;
                    }
                }
                if (globalSetting != nullptr) { //Need to add global upgrades
                    int j = 0;
                    for (const Property& prop : setting) {
                        file.push_back({ settingType + settingName,prop.name,"","",prop.value });
                        for (const Upgrade& upg : prop) {
                            file.push_back({ settingType + settingName,prop.name,upg.adj,upg.name,upg.value });
                        }
                        for (const Upgrade& upg : globalSetting->at(j)) {
                            file.push_back({ settingType + settingName,prop.name,upg.adj,upg.name,upg.value });
                        }
                        j++;
                    }
                }
                else { //Only local upgrades
                    for (const Property& prop : setting) {
                        file.push_back({ settingType + settingName,prop.name,"","",prop.value });
                        for (const Upgrade& upg : prop) {
                            file.push_back({ settingType + settingName,prop.name,upg.adj,upg.name,upg.value });
                        }
                    }
                }
            }
        }

        //Convert strings into binary
        vector<unsigned char> parseData;
        for (SettingLine line : file) {
            parseData.insert(parseData.end(), line.name.begin(), line.name.end());
            if (!line.prop.empty()) {
                parseData.insert(parseData.end(), SPLIT_USCORE.begin(), SPLIT_USCORE.end());
                parseData.insert(parseData.end(), line.prop.begin(), line.prop.end());
            }
            parseData.insert(parseData.end(), line.adj.begin(), line.adj.end());
            parseData.insert(parseData.end(), line.upg.begin(), line.upg.end());
            parseData.insert(parseData.end(), SPLIT_EQ.begin(), SPLIT_EQ.end());
            parseData.insert(parseData.end(), line.val.begin(), line.val.end());
            parseData.insert(parseData.end(), SPLIT_LINEEND.begin(), SPLIT_LINEEND.end());
        }
        size_t size = parseData.size();
        BinaryFile retval{ size, filename, SettingsExtension };
        memcpy(retval.begin(), parseData.data(), size * sizeof(unsigned char));
        return retval;
    }

    void Panel::UpdateData() {
        ref = data;
    }

    class PropGridData : public wxObject {
    public:
        PropGridData(Upgrade* upg, Property* parent) : wxObject{}, upg{ upg }, prop{ parent }, upgrade{ true } {}
        PropGridData(Property* prop) : wxObject{}, upg{ nullptr }, prop{ prop }, upgrade{ false } {}
        void SetValue(string value);
        Property* GetParent();
        Property* GetProperty();
        Upgrade* GetUpgrade();
        const bool IsUpgrade() const { return upgrade; }
    private:
        Upgrade* upg;
        Property* prop;
        bool upgrade;
    };

    void PropGridData::SetValue(string value) {
        if (upgrade) {
            upg->value = value;
        }
        else {
            prop->value = value;
        }
    }

    Property* PropGridData::GetParent() {
        if (upgrade) {
            return prop;
        }
        else {
            return nullptr;
        }
    }

    Property* PropGridData::GetProperty() {
        if (upgrade) {
            return nullptr;
        }
        else {
            return prop;
        }
    }

    Upgrade* PropGridData::GetUpgrade() {
        if (upgrade) {
            return upg;
        }
        else {
            return nullptr;
        }
    }

    class SGBoolProperty : public wxBoolProperty, public PropGridData {
    public:
        SGBoolProperty(Upgrade* upg, Property* parent) : wxBoolProperty{ upg->adj + upg->name }, PropGridData{ upg, parent }{ SetEditor(wxPGEditor_CheckBox); SetValueFromString(upg->value); }
        SGBoolProperty(Property* prop) : wxBoolProperty{ prop->name }, PropGridData{ prop }{ SetEditor(wxPGEditor_CheckBox); SetValueFromString(prop->value); }
    };

    class SGIntProperty : public wxIntProperty, public PropGridData {
    public:
        SGIntProperty(Upgrade* upg, Property* parent) : wxIntProperty{ upg->adj + upg->name }, PropGridData{ upg, parent }{ SetValueFromString(upg->value); }
        SGIntProperty(Property* prop) : wxIntProperty{ prop->name }, PropGridData{ prop }{ SetValueFromString(prop->value); }
    };

    class SGFloatProperty : public wxFloatProperty, public PropGridData {
    public:
        SGFloatProperty(Upgrade* upg, Property* parent) : wxFloatProperty{ upg->adj + upg->name }, PropGridData{ upg, parent }{ SetValueFromString(upg->value); }
        SGFloatProperty(Property* prop) : wxFloatProperty{ prop->name }, PropGridData{ prop }{ SetValueFromString(prop->value); }
    };

    class SGStringProperty : public wxStringProperty, public PropGridData {
    public:
        SGStringProperty(Upgrade* upg, Property* parent) : wxStringProperty{ upg->adj + upg->name }, PropGridData{ upg, parent }{ SetValueFromString(upg->value); }
        SGStringProperty(Property* prop) : wxStringProperty{ prop->name }, PropGridData{ prop }{ SetValueFromString(prop->value); }
    };

    class SGPropertyCatagory : public wxPropertyCategory, public PropGridData {
    public:
        SGPropertyCatagory(Property* prop) : wxPropertyCategory{ prop->name }, PropGridData{ prop }{}
    };

    Panel::Panel(Data& data) : IPanel{}, data{ data }, ref{ data } {
        InitTreectrl();
        InitPropgrid();
        InitSizer();
    }

    void Panel::InitTreectrl() {
        treectrl = new wxTreeCtrl(this, ID_SettingsTree, wxDefaultPosition, wxSize(250, 200), wxTR_TWIST_BUTTONS | wxTR_NO_LINES | wxTR_HAS_BUTTONS | wxTR_EDIT_LABELS);
        treectrl->Freeze();
        treectrl->SetImageList(new wxImageList(16, 16));
        treectrl->GetImageList()->Add(wxBitmap("Textures/settingtab_tree_root.png", wxBITMAP_TYPE_PNG));
        treectrl->GetImageList()->Add(wxBitmap("Textures/settingtab_tree_node.png", wxBITMAP_TYPE_PNG));
        treectrl->GetImageList()->Add(wxBitmap("Textures/settingtab_tree_node_lock.png", wxBITMAP_TYPE_PNG));
        treectrl->GetImageList()->Add(wxBitmap("Textures/settingtab_tree_leaf.png", wxBITMAP_TYPE_PNG));
        treectrl->GetImageList()->Add(wxBitmap("Textures/settingtab_tree_leaf_lock.png", wxBITMAP_TYPE_PNG));
        treectrl->GetImageList()->Add(wxBitmap("Textures/settingtab_tree_leaf_list.png", wxBITMAP_TYPE_PNG));
        auto root = treectrl->AddRoot(data.GetName(), 0, -1, new SettingsTreeData());
        if (data.md->IsGlobal()) {
            for (const Setting& setting : data) {
                treectrl->AppendItem(root, setting.catagory->name, 4, -1, new SettingsTreeData(setting.catagory, &setting, true));
            }
            treectrl->Expand(root);
        }
        else {
            auto it = data.begin();
            if (it->catagory == nullptr) {
                treectrl->AppendItem(root, "<NO_TYPE>", 5, -1, new SettingsTreeData(it->catagory, &*it));
                it++;
            }
            for (auto& cat : *(data.md)) {
                auto node = treectrl->AppendItem(root, cat.name, cat.locked ? 2 : 1, -1, new SettingsTreeData(&cat,cat.locked));
                if (it == data.end()) continue;
                while(it->catagory == &cat) {
                    if (it->IsEditable()) { //editable leaf icon
                        if (it->name.empty()) {
                            treectrl->AppendItem(node, "<LIST>", 5, -1, new SettingsTreeData(&cat,&*it,cat.locked));
                            it++;
                        }
                        else {
                            treectrl->AppendItem(node, it->name, 4, -1, new SettingsTreeData(&cat, &*it, cat.locked));
                            it++;
                        }
                    }
                    else if (cat.locked) { //locked leaf icon
                        if (it->name.empty()) {
                            treectrl->AppendItem(node, "<SPECIAL>", 4, -1, new SettingsTreeData(&cat, &*it, cat.locked));
                            it++;
                        }
                        else {
                            treectrl->AppendItem(node, it->name, 4, -1, new SettingsTreeData(&cat, &*it, cat.locked));
                            it++;
                        }
                    }
                    else { //normal leaf icon
                        treectrl->AppendItem(node, it->name, 3, -1, new SettingsTreeData(&cat, &*it, cat.locked));
                        it++;
                    }
                    if (it == data.end()) break;
                }
            }
            treectrl->Expand(root);
        }
        treectrl->Bind(wxEVT_TREE_ITEM_ACTIVATED, &Panel::OnTreeClick, this, ID_SettingsTree);
        treectrl->Bind(wxEVT_TREE_ITEM_MENU, &Panel::OnTreeItemMenu, this, ID_SettingsTree);
        treectrl->Bind(wxEVT_TREE_BEGIN_LABEL_EDIT, &Panel::OnTreeBeginLabelEdit, this);
        treectrl->Bind(wxEVT_TREE_END_LABEL_EDIT, &Panel::OnTreeEndLabelEdit, this);
        treectrl->Thaw();
    }

    void Panel::InitPropgrid() {
        propgrid = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
        propgrid->SetBoolChoices("true", "false");
        propgrid->Bind(wxEVT_PG_CHANGED, &Panel::OnPropChanged, this);
        propgrid->Bind(wxEVT_PG_RIGHT_CLICK, &Panel::OnPropItemMenu, this);
    }

    void Panel::InitSizer() {
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        sizer->Add(treectrl, 0, wxEXPAND);
        sizer->Add(propgrid, 1, wxEXPAND);
        SetSizerAndFit(sizer);
    }

    void Panel::OnTreeClick(wxTreeEvent& event) {
        SettingsTreeData* std = dynamic_cast<SettingsTreeData*>(treectrl->GetItemData(event.GetItem()));
        if (std->HasSetting()) {
            SetPropGrid(std->GetSetting(), event.GetItem()); 
        }
        else if (treectrl->GetRootItem() != event.GetItem()) {
            treectrl->Toggle(event.GetItem());
        }
    }

    void Panel::OnTreeItemMenu(wxTreeEvent& event) {
        treectrl->SelectItem(event.GetItem());
        SettingsTreeData* std = dynamic_cast<SettingsTreeData*>(treectrl->GetItemData(event.GetItem()));
        wxMenu menu;
        if (std->HasRenameMenu()) {
            menu.Append(ID_SettingsTreeMenuRename, "Rename");
            menu.Bind(wxEVT_MENU, &Panel::OnTreeMenuRename, this, ID_SettingsTreeMenuRename, -1, event.Clone());
        }
        if (std->HasDeleteMenu()) {
            menu.Append(ID_SettingsTreeMenuDelete, "Delete Setting");
            menu.Bind(wxEVT_MENU, &Panel::OnTreeMenuDelete, this, ID_SettingsTreeMenuDelete, -1, event.Clone());
        }
        if (std->HasAddSettingMenu()) {
            menu.Append(ID_SettingsTreeMenuNew, "New Setting...");
            menu.Bind(wxEVT_MENU, &Panel::OnTreeMenuNew, this, ID_SettingsTreeMenuNew, -1, event.Clone());
        }
        if (std->HasAddPropertyMenu()) {
            menu.Append(ID_SettingsTreeMenuAddProp, "Add Property...");
            menu.Bind(wxEVT_MENU, &Panel::OnTreeMenuAddProp, this, ID_SettingsTreeMenuAddProp, -1, event.Clone());
        }
        if (std->HasRenameMenu() || std->HasDeleteMenu() || std->HasAddSettingMenu() || std->HasAddPropertyMenu()) {
            menu.AppendSeparator();
        }
        menu.Append(ID_SettingsTreeMenuExpand, "Expand All");
        menu.Bind(wxEVT_MENU, &Panel::OnTreeMenuExpand, this, ID_SettingsTreeMenuExpand, -1, event.Clone());
        menu.Append(ID_SettingsTreeMenuCollapse, "Collapse All");
        menu.Bind(wxEVT_MENU, &Panel::OnTreeMenuCollapse, this, ID_SettingsTreeMenuCollapse, -1, event.Clone());
        PopupMenu(&menu);
    }

    void Panel::OnTreeMenuNew(wxCommandEvent& event) {
        wxTreeItemId item = dynamic_cast<wxTreeEvent*>(event.GetEventUserData())->GetItem();
        SettingsTreeData* std = dynamic_cast<SettingsTreeData*>(treectrl->GetItemData(item));
        SettingDialog* txtDiag = new SettingDialog(this, "Enter the name of the new setting:", data.GetName() + " : Add Setting", &data, std->GetSettingType());
        if (txtDiag->ShowModal() == wxID_OK) {
            string name = txtDiag->GetValue();
            Setting* setting = const_cast<Setting*>(&(*data.insert(Setting(name, std->GetSettingType()->defaultValue, std->GetSettingType())).first));
            wxTreeItemId newItem = treectrl->AppendItem(item, name, 3, -1, new SettingsTreeData(std->GetSettingType(), setting, std->GetSettingType()->special));
            treectrl->SortChildren(item);
            treectrl->Expand(item);
            treectrl->SelectItem(newItem);
            SetPropGrid(setting, newItem);
            SetEdited();
        }
        txtDiag->Destroy();
    }

    void Panel::OnTreeMenuRename(wxCommandEvent& event) {
        wxTreeItemId item = dynamic_cast<wxTreeEvent*>(event.GetEventUserData())->GetItem();
        wxTextCtrl* textctrl = treectrl->EditLabel(item);
        wxTextValidator valid{ wxFILTER_EXCLUDE_CHAR_LIST };
        valid.AddCharExcludes(invalidChar);
        textctrl->SetValidator(valid);
    }

    void Panel::OnTreeMenuDelete(wxCommandEvent& event) {
        wxTreeItemId item = dynamic_cast<wxTreeEvent*>(event.GetEventUserData())->GetItem();
        data.erase(*dynamic_cast<SettingsTreeData*>(treectrl->GetItemData(item))->GetSetting());
        if (treectrl->GetSelection() == activeGrid) {
            propgrid->Clear();
        }
        treectrl->Delete(item);
        SetEdited();
    }

    void Panel::OnTreeMenuAddProp(wxCommandEvent& event) {
        wxTreeItemId item = dynamic_cast<wxTreeEvent*>(event.GetEventUserData())->GetItem();
        SettingsTreeData* std = dynamic_cast<SettingsTreeData*>(treectrl->GetItemData(item));
        PropDialog diag{ this, data.GetName() + " : Add Property", "Enter the name of the new property:", std->GetSetting() };
        if (diag.ShowModal() == wxID_OK) {
            std->GetSetting()->push_back(Property(Property::Type::Str, diag.GetPropName(), ""));
            SetPropGrid(std->GetSetting(), item);
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
        SettingsTreeData* data = dynamic_cast<SettingsTreeData*>(treectrl->GetItemData(event.GetItem()));
        if (!data->HasRenameMenu()) {
            event.Veto();
        }
    }

    void Panel::OnTreeEndLabelEdit(wxTreeEvent& event) {
        string name = event.GetLabel().ToStdString();
        SettingsTreeData* std = dynamic_cast<SettingsTreeData*>(treectrl->GetItemData(event.GetItem()));
        if (name == "" || !std->HasSetting()) {
            event.Veto();
        }
        
        else if ((find(data.begin(), data.end(), Setting(name, {}, std->GetSettingType())) != data.end()) && (_stricmp(name.c_str(), treectrl->GetItemText(event.GetItem()).ToStdString().c_str()) != 0)) {
            wxMessageBox("Could not rename setting: Name already in use!", "Error : Invalid value", wxOK | wxICON_ERROR);
            event.Veto();
        }
        else {
            auto it = data.extract(*(std->GetSetting()));
            it.value().name = name;
            data.insert(move(it));
            treectrl->SetItemText(event.GetItem(), event.GetLabel());
            treectrl->SortChildren(treectrl->GetItemParent(event.GetItem()));
            SetEdited();
        }
    }

    void Panel::SetPropGrid(Setting* setting, wxTreeItemId item) {
        propgrid->Freeze();
        propgrid->Clear();
        for (Property& prop : *setting) {
            auto node = propgrid->Append(CreatePGItem(prop));
            for (const Upgrade& upg : prop) {
                propgrid->AppendIn(node, CreatePGItem(prop, upg));
            }
        }
        if (setting->catagory == nullptr) {
            propgrid->CollapseAll();
        }
        else {
            propgrid->ExpandAll();
        }
        propgrid->Thaw();
        propgrid->FitColumns();
        activeGrid = item;
    }

    wxPGProperty* Panel::CreatePGItem(Property& prop) {
        if (data.md->IsGlobal()) {
            return new SGPropertyCatagory(&prop);
        }
        else {
            switch (prop.type) {
            case Property::Type::Bool:
                return new SGBoolProperty(&prop);
            case Property::Type::Int:
                return new SGIntProperty(&prop);
            case Property::Type::Dbl:
                return new SGFloatProperty(&prop);
            default:
                return new SGStringProperty(&prop);
            }
        }
    }

    wxPGProperty* Panel::CreatePGItem(Property& prop, const Upgrade& upg) {
        switch (prop.type) {
        case Property::Type::Bool:
            return new SGBoolProperty(const_cast<Upgrade*>(&upg), &prop);
        case Property::Type::Int:
            return new SGIntProperty(const_cast<Upgrade*>(&upg), &prop);
        case Property::Type::Dbl:
            return new SGFloatProperty(const_cast<Upgrade*>(&upg), &prop);
        default:
            return new SGStringProperty(const_cast<Upgrade*>(&upg), &prop);
        }
    }

    void Panel::OnPropChanged(wxPropertyGridEvent& event) {
        if (event.GetProperty() != NULL) {
            if (event.GetProperty()->GetEditorClass()->GetName() == "CheckBox") {
                if (event.GetValue().GetInteger() == 1) {
                    dynamic_cast<PropGridData*>(event.GetProperty())->SetValue("true");
                }
                else {
                    dynamic_cast<PropGridData*>(event.GetProperty())->SetValue("false");
                }
            }
            else {
                dynamic_cast<PropGridData*>(event.GetProperty())->SetValue(event.GetValue().MakeString().ToStdString());
            }
            SetEdited();
        }
    }

    void Panel::OnPropItemMenu(wxPropertyGridEvent& event) {
        PropGridData* data = dynamic_cast<PropGridData*>(event.GetProperty());
        wxMenu menu;
        if (data->IsUpgrade()) {
            menu.Append(ID_SettingsPropMenuRename, "Rename Upgrade...");
            menu.Bind(wxEVT_MENU, &Panel::OnPropMenuRename, this, ID_SettingsPropMenuRename, -1, event.Clone());
            menu.Append(ID_SettingsPropMenuDelete, "Delete Upgrade");
            menu.Bind(wxEVT_MENU, &Panel::OnPropMenuDelete, this, ID_SettingsPropMenuDelete, -1, event.Clone());
        }
        else {
            menu.Append(ID_SettingsPropMenuNew, "New Upgrade...");
            menu.Bind(wxEVT_MENU, &Panel::OnPropMenuNew, this, ID_SettingsPropMenuNew, -1, event.Clone());
            if (dynamic_cast<SettingsTreeData*>(treectrl->GetItemData(activeGrid))->GetSetting()->IsEditable()) {
                menu.Append(ID_SettingsPropMenuRemoveClass, "Remove Property");
                menu.Bind(wxEVT_MENU, &Panel::OnPropMenuRemoveProp, this, ID_SettingsPropMenuRemoveClass, -1, event.Clone());
            }
        }
        menu.AppendSeparator();
        menu.Append(ID_SettingsPropMenuExpand, "Expand All");
        menu.Bind(wxEVT_MENU, &Panel::OnPropMenuExpand, this, ID_SettingsPropMenuExpand);
        menu.Append(ID_SettingsPropMenuCollapse, "Collapse All");
        menu.Bind(wxEVT_MENU, &Panel::OnPropMenuCollapse, this, ID_SettingsPropMenuCollapse);
        PopupMenu(&menu);
        event.Skip();
    }

    void Panel::OnPropMenuNew(wxCommandEvent& event) {
        wxPropertyGridEvent* oldEvent = dynamic_cast<wxPropertyGridEvent*>(event.GetEventUserData());
        Property* prop = dynamic_cast<PropGridData*>(oldEvent->GetProperty())->GetProperty();
        UpgradeDialog* upgDiag = new UpgradeDialog(propgrid, "Add Upgrade : " + prop->name, "Enter the name of the new upgrade:", prop, data.md->GetAdj());
        if (upgDiag->ShowModal() == wxID_OK) {
            const Upgrade& upg = *(prop->insert(Upgrade(upgDiag->getAdj(), upgDiag->getName(), prop->DefaultVal())).first);
            propgrid->Freeze();
            propgrid->AppendIn(oldEvent->GetProperty(), CreatePGItem(*prop, upg));
            propgrid->SortChildren(oldEvent->GetProperty());
            propgrid->Thaw();
            SetEdited();
        }
        upgDiag->Destroy();
    }

    void Panel::OnPropMenuRename(wxCommandEvent& event) {
        wxPropertyGridEvent* oldEvent = dynamic_cast<wxPropertyGridEvent*>(event.GetEventUserData());
        PropGridData* pgd = dynamic_cast<PropGridData*>(oldEvent->GetProperty());
        UpgradeDialog* upgDiag = new UpgradeDialog(propgrid, "Rename Upgrade : " + pgd->GetUpgrade()->adj + pgd->GetUpgrade()->name, "Enter the new name of the upgrade:", pgd->GetParent(), data.md->GetAdj(), pgd->GetUpgrade()->adj, pgd->GetUpgrade()->name);
        if (upgDiag->ShowModal() == wxID_OK) {
            pgd->GetUpgrade()->adj = upgDiag->getAdj();
            pgd->GetUpgrade()->name = upgDiag->getName();
            propgrid->Freeze();
            propgrid->SetPropertyName(oldEvent->GetProperty(), upgDiag->getAdj() + upgDiag->getName());
            propgrid->SetPropertyLabel(oldEvent->GetProperty(), upgDiag->getAdj() + upgDiag->getName());
            propgrid->SortChildren(oldEvent->GetProperty()->GetParent());
            propgrid->Thaw();
            SetEdited();
        }
        upgDiag->Destroy();
    }

    void Panel::OnPropMenuDelete(wxCommandEvent& event) {
        wxPropertyGridEvent* oldEvent = dynamic_cast<wxPropertyGridEvent*>(event.GetEventUserData());
        PropGridData* pgd = dynamic_cast<PropGridData*>(oldEvent->GetProperty());
        pgd->GetParent()->erase(*pgd->GetUpgrade());
        propgrid->RemoveProperty(oldEvent->GetProperty());
        SetEdited();
    }

    void Panel::OnPropMenuRemoveProp(wxCommandEvent& event) {
        wxPropertyGridEvent* oldEvent = dynamic_cast<wxPropertyGridEvent*>(event.GetEventUserData());
        PropGridData* pgd = dynamic_cast<PropGridData*>(oldEvent->GetProperty());
        Setting* setting = dynamic_cast<SettingsTreeData*>(treectrl->GetItemData(activeGrid))->GetSetting();
        setting->erase(find(setting->begin(), setting->end(), *(pgd->GetProperty())));
        propgrid->DeleteProperty(oldEvent->GetProperty());
        SetEdited();
    }

    void Panel::OnPropMenuExpand(wxCommandEvent& event) {
        propgrid->ExpandAll();
    }

    void Panel::OnPropMenuCollapse(wxCommandEvent& event) {
        propgrid->CollapseAll();
    }
}