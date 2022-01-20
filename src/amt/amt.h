/*
* Creates the main window and common tabs in the program.
* The program has a window called "MainFrame" as follows:
* 
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++
* + File toolbar & icons                                +
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++
* + Tab 1 | Tab 2 | TAB 3 |                             +
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++
* + Frame for tab 3                    | Tree           +
* +                                    | -child folder1 +
* +                                    | --child node1  +
* +                                    | --child node2  +
* +                                    | -child folder2 +
* +                                    | ...            +
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++
* 
* All files are loaded at program launch.
*/

#pragma once

#include <wx/wxprec.h>
#include <vector>
#include <string>
#include <memory>
#include <thread>

#include "global.h"
#include "framework.h"

//wxWidgets event declaration
wxDECLARE_EVENT(wxEVT_LOADING_PROGRESS, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_LOADING_COMPLETE, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_LOADING_FAILED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SAVING_COMPLETE, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SAVING_FAILED, wxCommandEvent);

//Forward declaration of classes
class wxAuiNotebook;
class wxTreeCtrl;
class wxTreeItemId;
class wxTreeEvent;
class wxStyledTextCtrl;
class wxStyledTextEvent;
class wxAuiNotebookEvent;

namespace amt {
    //FRAMES---------------------------------------------------------------------------------------------------------------------------------------------

    class wxMainTreeData;

    //Main Program window: 
    //-treectrl is the right hand tree
    //-notebook is controls the the tabs and contains the frames
    //-active tab holds a reference to the currently selected frame
    class MainFrame : public wxFrame {
    public:
        MainFrame();
        wxAuiNotebook* GetNotebook() const { return notebook; }
        void RefreshTreectrl();
    private:
        wxTreeCtrl* treectrl;
        wxAuiNotebook* notebook;
        std::map<wxMainTreeData*, IPanel*> activeTab;
        void InitMenubar();
        void InitToolbar();
        void InitStatusbar();
        void InitTreectrl();
        void InitNotebook();
        void InitSizer();
        void GenTreeCtrl(wxTreeItemId& id, TreeMap& tm);
        void OnMousewheel(wxMouseEvent& event);
        void OnMenuExit(wxCommandEvent& event);
        void OnMenuAbout(wxCommandEvent& event);
        void OnMenuSave(wxCommandEvent& event);
        void OnMenuSaveAll(wxCommandEvent& event);
        void OnTreeClick(wxTreeEvent& event);
        void OnTreeMenu(wxTreeEvent& event);
        void OnTreeMenuDisplay(wxCommandEvent& event);
        void OnTreeMenuAddLeaf(wxCommandEvent& event);
        void OnTreeMenuRename(wxCommandEvent& event);
        void OnTreeBeginLabelEdit(wxTreeEvent& event);
        void OnTreeEndLabelEdit(wxTreeEvent& event);
        void OnTreeMenuDelete(wxCommandEvent& event);
        void OnTreeMenuExpand(wxCommandEvent& event);
        void OnTreeMenuCollapse(wxCommandEvent& event);
        void OnNotebookPageClose(wxAuiNotebookEvent& event);
        void OnNotebookPageChanged(wxAuiNotebookEvent& event);
        void OnPlaceholder(wxCommandEvent& event);
    };

    //PANELS---------------------------------------------------------------------------------------------------------------------------------------------

    //Panel interface that forces tabs to handle events passed from MainFrame to the tabs to prevent propagating upwards into a loop
    class IPanel : public wxPanel {
        friend class MainFrame;
    public:
        IPanel();
    protected:
        void SetEdited(); //Derived classes should call whenever a saveable change is made
        virtual std::string GetTabName() const = 0;
        virtual void UpdateData() = 0; //Called before save, Derived classes should make the model's data up to date
    private:
        bool saved{ true };
        std::string tabId;
        void InitPanelEvents(); //MainFrame call
        void GenPanelEvents(wxWindowList& winlist); //Recursive helper
        bool IsSaved() const { return saved; } //MainFrame call
        void HandleSave(); //MainFrame call
        void OnMousewheel(wxMouseEvent& event);
        void OnChildMousewheel(wxMouseEvent& event);

    };

    //APP------------------------------------------------------------------------------------------------------------------------------------------------

    //wxWidgets App; replaces the main function, holds unique pointers to
    //all of the parsed data
    class AmtApp : public wxApp {
    public:
        virtual bool OnInit() override;
        virtual int OnExit() override;
        std::vector<std::unique_ptr<IModel>>& GetModel() { return model; };
        std::string GetModName() const { return "Enviroment"; }
        const std::vector<Hero>& GetHeroList() const { return heroList; }
        MainFrame* GetMainFrame() { return frame; }
        bool IsHero(const std::string& name) { return heroList.end() != find_if(heroList.begin(), heroList.end(), [&name](const Hero& hero) {return _stricmp(hero.name.c_str(), name.c_str()) == 0; }); }
        void LoadEnviroment();
        void SaveEnviroment();
    private:
        MainFrame* frame;
        std::vector<std::unique_ptr<IModel>> model;
        std::vector<Hero> heroList;
        void InitModels();
        void SetLoadProgressEvent(wxEvtHandler* loadingBar, const std::string& text);
        void SetLoadCompleteEvent(wxEvtHandler* loadingBar);
        void SetLoadFailedEvent(wxEvtHandler* loadingBar, const std::string& error);
        void SetSaveCompleteEvent(wxEvtHandler* savingBar);
        void SetSaveFailedEvent(wxEvtHandler* savingBar, const std::string& error);
    };

    //COMMON---------------------------------------------------------------------------------------------------------------------------------------------

    //Stores the data of a file as a string
    class StringData : public amt::IData {
    public:
        StringData() = default;
        StringData(const std::string& key, std::string&& str) : amt::IData{ key }, _str{ str } {}
        StringData(const std::string& key, const std::string& str) : amt::IData{ key }, _str{ str } {}
        std::string& GetString() const { return _str; }
        void SetString(const std::string& str) const { _str = str; }
        void SetString(std::string&& str) const { _str = str; }
    private:
        mutable std::string _str;
    };

    //Copys the file data into a single string
    class IStringParser : public IParser<StringData> {
    public:
        IStringParser() = delete;
        IStringParser(const std::filesystem::path& directory, unsigned int mode = 0U, std::function<bool(const std::filesystem::path&)>&& predicate = [](const std::filesystem::path&) { return true; }) : IParser<StringData>{ directory,mode,move(predicate) } {}
        virtual apcl::BindableFolder Parse(const Container<StringData>& data) override;
        virtual Container<StringData> Parse(apcl::BindableFolder&& data) override;
    private:
        void StrParse(const Container<StringData>& data, apcl::BindableFolder& ctnr);
        void StrParse(apcl::BindableFolder& data, Container<StringData>& ctnr);
    };

    //Displays file information as plaintext: used to read the file before designing the parsing algorithm
    class StringPanel : public IPanel {
    public:
        StringPanel() = delete;
        StringPanel(const StringData& data);
    protected:
        virtual void UpdateData() override { ref.SetString(textctrl->GetValue().ToStdString()); };
        virtual std::string GetTabName() const override { return "Text - " + ref.GetName(); }
        const StringData& ref;
    private:
        wxTextCtrl* textctrl;
        void InitTextctrl();
        void InitSizer();
        void OnTextEdit(wxCommandEvent& event);
    };

    //Displays file information as plaintext with XML markup; the game contains a lot of XML files
    class XmlPanel : public IPanel {
    public:
        XmlPanel() = delete;
        XmlPanel(const StringData& data);
    protected:
        virtual void UpdateData() override;
        virtual std::string GetTabName() const override { return "XML - " + ref.GetName(); }
        const StringData& ref;
    private:
        wxStyledTextCtrl* textctrl;
        void InitTextctrl();
        void InitSizer();
        void OnTextEdit(wxStyledTextEvent& event);
    };
}

//Declaration of wxWidgets "main function"
wxDECLARE_APP(amt::AmtApp);