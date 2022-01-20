#pragma once

#include <string>
#include <list>
#include <vector>
#include <fstream>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/valnum.h>
#include <wx/treectrl.h>

#include "amt.h"

//The settings have been shoehorned from an old version of AMT into the new framework.
//As a result, this file is a complete mess and ideally would be completely rewritten.

namespace Settings {
    class Upgrade {
    public:
        Upgrade() = default;
        Upgrade(const std::string& adj, const std::string& name, const std::string& value) : adj{ adj }, name{ name }, value{ value } {};
        Upgrade(std::string&& adj, std::string&& name, std::string&& value) : adj{ adj }, name{ name }, value{ value } {};
        friend bool operator==(const Upgrade& lhs, const Upgrade& rhs) { return (lhs.adj == rhs.adj) && (_stricmp(lhs.name.c_str(), rhs.name.c_str()) == 0); }
        friend bool operator!=(const Upgrade& lhs, const Upgrade& rhs) { return !operator==(lhs, rhs); }
        friend bool operator< (const Upgrade& lhs, const Upgrade& rhs) { return (lhs.adj == rhs.adj) ? (_stricmp(lhs.name.c_str(), rhs.name.c_str()) < 0) : (lhs.adj < rhs.adj); }
        friend bool operator> (const Upgrade& lhs, const Upgrade& rhs) { return  operator< (rhs, lhs); }
        friend bool operator<=(const Upgrade& lhs, const Upgrade& rhs) { return !operator> (lhs, rhs); }
        friend bool operator>=(const Upgrade& lhs, const Upgrade& rhs) { return !operator< (lhs, rhs); }
        std::string adj;
        std::string name;
        std::string value;
    };

    class Property : public std::set<Upgrade> {
        friend class Parser;
    public:
        enum class Type { Bool, Int, Dbl, Str, Pair, Color };
        Property() = default;
        Property(Type type, const std::string& name, const std::string& defaultVal) : std::set<Upgrade>{}, type{ type }, name{ name }, value{ defaultVal }, defaultVal{ defaultVal } {}
        Property(Type type, const std::string& name) : std::set<Upgrade>{}, type{ type }, name{ name }, value{ DefaultOf(type) }, defaultVal{ DefaultOf(type) } {}
        const std::string& DefaultVal() const { return defaultVal; }
        std::string name;
        std::string value;
        Type type{ Type::Str };
        friend bool operator==(const Property& lhs, const Property& rhs) { return (_stricmp(lhs.name.c_str(), rhs.name.c_str()) == 0); }
        friend bool operator!=(const Property& lhs, const Property& rhs) { return !operator==(lhs, rhs); }
        friend bool operator< (const Property& lhs, const Property& rhs) { return (_stricmp(lhs.name.c_str(), rhs.name.c_str()) < 0); }
        friend bool operator> (const Property& lhs, const Property& rhs) { return  operator< (rhs, lhs); }
        friend bool operator<=(const Property& lhs, const Property& rhs) { return !operator> (lhs, rhs); }
        friend bool operator>=(const Property& lhs, const Property& rhs) { return !operator< (lhs, rhs); }
    private:
        std::string defaultVal;
        std::string DefaultOf(Type type);
    };

    struct Catagory {
        std::string name;
        std::vector<Property> defaultValue;
        bool special{ false };
        bool reversed{ false };
        bool locked{ false };
        friend bool operator==(const Catagory& lhs, const Catagory& rhs) { return (lhs.name == rhs.name); }
        friend bool operator!=(const Catagory& lhs, const Catagory& rhs) { return !operator==(lhs, rhs); }
        friend bool operator< (const Catagory& lhs, const Catagory& rhs) { return (lhs.name < rhs.name); }
        friend bool operator> (const Catagory& lhs, const Catagory& rhs) { return  operator< (rhs, lhs); }
        friend bool operator<=(const Catagory& lhs, const Catagory& rhs) { return !operator> (lhs, rhs); }
        friend bool operator>=(const Catagory& lhs, const Catagory& rhs) { return !operator< (lhs, rhs); }
    };

    class Metadata : public std::vector<Catagory> {
        inline static const std::vector<std::string> globalAdj{ "@@","++","**","~~" };
        inline static const std::vector<std::string> normalAdj{ "@" ,"+" ,"*" ,"~" };
    public:
        Metadata() = default;
        Metadata(bool global) : std::vector<Catagory>{}, global{ global } {}
        const std::vector<std::string>& GetAdj() const { return (global ? globalAdj : normalAdj); };
        bool IsGlobal() const { return global; }
        void SetGlobal() { global = true; }
    private:
        bool global{ false };
    };

    class Setting : public std::vector<Property> {
    public:
        Setting() = default;
        Setting(const std::string& name, const std::vector<Property>& properties, const Catagory* catagory = nullptr) : std::vector<Property>{ properties }, catagory{ catagory }, name{ name } {}
        Setting(const std::string& name, std::vector<Property>&& properties, const Catagory* catagory = nullptr) : std::vector<Property>{ move(properties) }, catagory{ catagory }, name{ name } {}
        std::string name;
        bool HasCatagory() const { return catagory != nullptr; }
        bool IsEditable() const { return (catagory ? (name.empty() && catagory->special) : true); };
        friend bool operator==(const Setting& lhs, const Setting& rhs) { return (_stricmp(lhs.name.c_str(), rhs.name.c_str()) == 0) && (lhs.catagory == rhs.catagory); }
        friend bool operator!=(const Setting& lhs, const Setting& rhs) { return !operator==(lhs, rhs); }
        friend bool operator< (const Setting& lhs, const Setting& rhs) { return (lhs.catagory == rhs.catagory) ? (_stricmp(lhs.name.c_str(), rhs.name.c_str()) < 0) : (lhs.catagory < rhs.catagory); }
        friend bool operator> (const Setting& lhs, const Setting& rhs) { return  operator< (rhs, lhs); }
        friend bool operator<=(const Setting& lhs, const Setting& rhs) { return !operator> (lhs, rhs); }
        friend bool operator>=(const Setting& lhs, const Setting& rhs) { return !operator< (lhs, rhs); }
        const Catagory* catagory{ nullptr };
    };

    class Data : public amt::IData, public std::set<Setting> {
    public:
        Data() = default;
        Data(const std::string& key, Metadata* md) : amt::IData{ key }, std::set<Setting>{}, md{ md } {}
        Metadata* md{ nullptr };
    };

    class Parser : public amt::IParser<Data> {
        inline static const std::string SPLIT_EQ = " = ";
        inline static const std::string SPLIT_USCORE = "_";
        inline static const std::string SPLIT_LINEEND = "\r\n";
        const std::vector<char> ADJ_CHAR = { '*','@','+','~' };
        inline static const std::string SpecialFileName{ "All" };
        inline static const std::string HeroFilePrefix{ "Settings" };
        inline static const std::string Global{ "Global" };
        inline static const std::filesystem::path MetaSubDir{ "Settings" };
        inline static const std::filesystem::path MetaTypeDir{ "Types" };
        inline static const std::string MetaExtension{ ".csv" };
        inline static const std::string SettingsExtension{ ".settings" };
    public:
        Parser();
        virtual void LoadMetadata() override;
        virtual amt::Container<Data> Parse(apcl::BindableFolder&& data) override;
        virtual apcl::BindableFolder Parse(const amt::Container<Data>& data) override;
    private:
        Metadata HeroMd;
        Metadata SpecialMd;
        Metadata GlobalMd;
        Data ParseFile(std::string filename, apcl::BinaryFile& bf, Metadata& md, std::vector<Setting>& globalSettings);
        apcl::BinaryFile ParseData(std::string filename, const Data& data, const Data& globalData);
        Metadata LoadMetadataFile(std::filesystem::path&& fp);
    };

    class Model : public amt::IModel {
    public:
        Model() : dm{ this } {}
        virtual std::string GetLoadingMessage() const override { return "Loading Settings..."; }
        virtual amt::TreeMap GetTreeMap() override;
    private:
        amt::DataManager<Data, Parser> dm;
    };

    class Panel : public amt::IPanel {
    public:
        Panel(Data& data);
        virtual std::string GetTabName() const override { return "Settings - " + ref.GetName(); }
        virtual void UpdateData() override;
    private:
        Data& ref;
        Data data;
        wxTreeCtrl* treectrl;
        wxPropertyGrid* propgrid;
        wxTreeItemId activeGrid;
        void InitTreectrl();
        void InitPropgrid();
        void InitSizer();
        wxPGProperty* CreatePGItem(Property& prop);
        wxPGProperty* CreatePGItem(Property& prop, const Upgrade& upg);
        void SetPropGrid(Setting* setting, wxTreeItemId item);
        void OnTreeClick(wxTreeEvent& event);
        void OnTreeItemMenu(wxTreeEvent& event);
        void OnTreeMenuNew(wxCommandEvent& event);
        void OnTreeMenuRename(wxCommandEvent& event);
        void OnTreeMenuDelete(wxCommandEvent& event);
        void OnTreeMenuAddProp(wxCommandEvent& event);
        void OnTreeMenuExpand(wxCommandEvent& event);
        void OnTreeMenuCollapse(wxCommandEvent& event);
        void OnTreeBeginLabelEdit(wxTreeEvent& event);
        void OnTreeEndLabelEdit(wxTreeEvent& event);
        void OnPropChanged(wxPropertyGridEvent& event);
        void OnPropItemMenu(wxPropertyGridEvent& event);
        void OnPropMenuNew(wxCommandEvent& event);
        void OnPropMenuRename(wxCommandEvent& event);
        void OnPropMenuDelete(wxCommandEvent& event);
        void OnPropMenuRemoveProp(wxCommandEvent& event);
        void OnPropMenuExpand(wxCommandEvent& event);
        void OnPropMenuCollapse(wxCommandEvent& event);
    };

    class SettingDialog : public wxDialog {
    public:
        SettingDialog(wxWindow* parent, const std::string& title, const std::string& caption, Data* settings, Catagory* settingType);
        std::string GetValue() const { return name; }
        void OnOk(wxCommandEvent& event);
        void OnTextEnter(wxCommandEvent& event);
    private:
        Data* settings;
        Catagory* settingType;
        wxTextCtrl* textctrl;
        std::string name;
    };

    class PropDialog : public wxDialog {
    public:
        PropDialog(wxWindow* parent, const std::string& title, const std::string& caption, Setting* setting, const std::string& propName = "");
        std::string GetPropName() const { return propName; }
        void OnOk(wxCommandEvent& event);
        void OnTextEnter(wxCommandEvent& event);
    private:
        Setting* setting;
        wxTextCtrl* textctrl;
        std::string propName;
    };

    class UpgradeDialog : public wxDialog {
    public:
        UpgradeDialog(wxWindow* parent, const std::string& title, const std::string& caption, Property* prop, const std::vector<std::string>& adjVect, const std::string& adj = "", const std::string& name = "");
        std::string getAdj() const { return adj; }
        std::string getName() const { return name; }
        void OnOk(wxCommandEvent& event);
        void OnTextEnter(wxCommandEvent& event);
    private:
        std::string adj;
        std::string name;
        wxComboBox* combobox;
        wxTextCtrl* textctrl;
        Property* prop;
        std::string defaultAdj;
        std::string defaultName;
    };
}