#include <wx/progdlg.h>
#include <wx/treectrl.h>
#include <wx/aui/auibook.h>
#include <wx/stc/stc.h>

#include "amt.h"

//Definition of wxWidgets events
wxDEFINE_EVENT(wxEVT_LOADING_PROGRESS, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_LOADING_COMPLETE, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_LOADING_FAILED, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SAVING_COMPLETE, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_SAVING_FAILED, wxCommandEvent);

using namespace std;
using namespace std::filesystem;
using namespace apcl;

namespace amt {
    //DIALOGS--------------------------------------------------------------------------------------------------------------------------------------------

    //Dialog that shows a loading progress bar when loading all of the files
    class LoadingDialog : public wxProgressDialog {
    public:
        LoadingDialog(wxWindow* parent, int max);
    private:
        void OnProgress(wxCommandEvent& event);
        void OnComplete(wxCommandEvent& event);
        void OnFailed(wxCommandEvent& event);
    };

    LoadingDialog::LoadingDialog(wxWindow* parent, int max) : wxProgressDialog{ "Loading AMT", "Initializing", max + 1, parent, wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_AUTO_HIDE } {
        Bind(wxEVT_LOADING_PROGRESS, &LoadingDialog::OnProgress, this, wxID_ANY);
        Bind(wxEVT_LOADING_COMPLETE, &LoadingDialog::OnComplete, this, wxID_ANY);
        Bind(wxEVT_LOADING_FAILED, &LoadingDialog::OnFailed, this, wxID_ANY);
    }

    void LoadingDialog::OnProgress(wxCommandEvent& event) {
        if (!Update(GetValue() + 1, event.GetString())) {
            wxTheApp->Exit();
        }
    }

    void LoadingDialog::OnComplete(wxCommandEvent& event) {
        Update(m_maximum, "Loading Complete");
    }

    void LoadingDialog::OnFailed(wxCommandEvent& event) {
        wxMessageBox("An error occured whilst loading data:\n" + event.GetString() + "\nThe enviroment may be corrupt; the program will now exit.", "Error", wxICON_ERROR);
        wxTheApp->Exit();
    }

    //Dialog that shows a loading progress bar when saving all of the files
    class SavingDialog : public wxProgressDialog {
    public:
        SavingDialog(wxWindow* parent);
    private:
        void OnComplete(wxCommandEvent& event);
        void OnFailed(wxCommandEvent& event);
    };

    SavingDialog::SavingDialog(wxWindow* parent) : wxProgressDialog{ "AMT : Saving", "Saving...", 1, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE } {
        Pulse("Saving data...");
        Bind(wxEVT_SAVING_COMPLETE, &SavingDialog::OnComplete, this);
        Bind(wxEVT_SAVING_FAILED, &SavingDialog::OnFailed, this);
    }

    void SavingDialog::OnComplete(wxCommandEvent& event) {
        Update(m_maximum, "Save complete");
    }

    void SavingDialog::OnFailed(wxCommandEvent& event) {
        wxMessageBox("An error occured whilst saving data:\n" + event.GetString() + "\nSome data may not be saved, ensure you have space and try again.", "Error", wxICON_ERROR);
        Update(m_maximum, "Save failed");
    }

    //AMT APP--------------------------------------------------------------------------------------------------------------------------------------------

    bool AmtApp::OnInit() {
        wxImage::AddHandler(new wxPNGHandler);
        InitModels();
        frame = new MainFrame();
        frame->Show(true);
        LoadEnviroment();
        return true;
    }

    int AmtApp::OnExit() {
        return 0;
    }

    void AmtApp::InitModels() {
        model = GetModelList();
    }

    void AmtApp::LoadEnviroment() {
        LoadingDialog* loadingBar = new LoadingDialog(frame, model.size());
		thread loadingThread([this, loadingBar]() {
			try {
                heroList = Hero::LoadHeroList();
				for (auto& m : model) {
                    SetLoadProgressEvent(loadingBar, m->GetLoadingMessage());
					m->Load();
				}
				SetLoadCompleteEvent(loadingBar);
                frame->RefreshTreectrl();
			}
			catch (FileException& e) {
				SetLoadFailedEvent(loadingBar, e.what());
			}
            catch (InterfaceException& e) {
                SetLoadFailedEvent(loadingBar, e.what());
            }
		});
		loadingThread.detach();
    }

    void AmtApp::SaveEnviroment() {
        SavingDialog* savingBar = new SavingDialog(frame);
        thread savingThread([this, savingBar]() {
            try {
                for (auto& m : model) {
                    m->Save();
                }
                SetSaveCompleteEvent(savingBar);
            }
            catch (FileException& e) {
                SetSaveFailedEvent(savingBar, e.what());
            }
        });
        savingThread.detach();
    }

	void AmtApp::SetLoadProgressEvent(wxEvtHandler* loadingBar, const string& text) {
		wxCommandEvent* event = new wxCommandEvent(wxEVT_LOADING_PROGRESS);
		event->SetString(text);
		wxQueueEvent(loadingBar, event);
	}

	void AmtApp::SetLoadCompleteEvent(wxEvtHandler* loadingBar) {
		wxCommandEvent* event = new wxCommandEvent(wxEVT_LOADING_COMPLETE);
		wxQueueEvent(loadingBar, event);
	}

	void AmtApp::SetLoadFailedEvent(wxEvtHandler* loadingBar, const string& error) {
		wxCommandEvent* event = new wxCommandEvent(wxEVT_LOADING_FAILED);
		event->SetString(error);
		wxQueueEvent(loadingBar, event);
	}

    void AmtApp::SetSaveCompleteEvent(wxEvtHandler* savingBar) {
        wxCommandEvent* event = new wxCommandEvent(wxEVT_SAVING_COMPLETE);
        wxQueueEvent(savingBar, event);
    }

    void AmtApp::SetSaveFailedEvent(wxEvtHandler* savingBar, const string& error) {
        wxCommandEvent* event = new wxCommandEvent(wxEVT_SAVING_FAILED);
        event->SetString(error);
        wxQueueEvent(savingBar, event);
    }

    //WX DATA OBJECTS------------------------------------------------------------------------------------------------------------------------------------

    class wxMainTreeData : public wxTreeItemData, public TreeNode { //Holds data for MainPanel's treectrl
    public:
        wxMainTreeData() = default;
        wxMainTreeData(TreeNode&& node) : TreeNode{ move(node) } {}
    };

    class wxWindowObject : public wxObject { //Holds a window pointer so that it can be Binded to an event
    public:
        wxWindowObject(wxWindow* window) : wxObject{}, window{ window } {}
        wxWindow* window;
    };

    //MAIN FRAME-----------------------------------------------------------------------------------------------------------------------------------------

    MainFrame::MainFrame() : wxFrame{ NULL, wxID_ANY, "Awesomenauts Modding Tookit", wxDefaultPosition, wxSize(1000, 700) } {
        Center();
        InitMenubar();
        InitToolbar();
        InitStatusbar();
        InitTreectrl();
        InitNotebook();
        InitSizer();
        SetMinSize(wxSize(1000, 700));
        Maximize();
        Raise();
        SetFocus();
        Show();
    }

    void MainFrame::InitMenubar() {
        wxMenu* menuFile = new wxMenu;
        menuFile->Append(ID_New, "&New\tCtrl-N", "Create a new mod in a fresh enviroment");
        menuFile->Append(ID_Open, "&Open...\tCtrl-O", "Load an existing AML file to a fresh enviroment");
        menuFile->AppendSeparator();
        menuFile->Append(ID_Save, "&Save\tCtrl-S", "Save the changes in the active tab to the modding enviroment");
        menuFile->Append(ID_SaveAll, "Save &All\tCtrl-Shift-S", "Save all changes to the modding enviroment");
        menuFile->Append(ID_Export, "&Export\tCtrl-E", "Save all changes to the modding enviroment and update the AML file");
        menuFile->Append(ID_ExportAs, "&Export As...\tCtrl-Shift-E", "Saves all changes to the modding enviroment and save to an AML file");
        menuFile->AppendSeparator();
        menuFile->Append(wxID_EXIT);
        wxMenu* menuHelp = new wxMenu;
        menuHelp->Append(wxID_ABOUT);
        wxMenuBar* menuBar = new wxMenuBar;
        menuBar->Append(menuFile, "&File");
        menuBar->Append(menuHelp, "&Help");
        SetMenuBar(menuBar);
        Bind(wxEVT_MENU, &MainFrame::OnMenuSave, this, ID_Save);
        Bind(wxEVT_MENU, &MainFrame::OnMenuSaveAll, this, ID_SaveAll);
        Bind(wxEVT_MENU, &MainFrame::OnMenuExit, this, wxID_EXIT);
        Bind(wxEVT_MENU, &MainFrame::OnMenuAbout, this, wxID_ABOUT);
    }

    void MainFrame::InitToolbar() {
        wxBitmap newIcon("Textures/iconbar_new.png", wxBITMAP_TYPE_PNG);
        wxBitmap openIcon("Textures/iconbar_open.png", wxBITMAP_TYPE_PNG);
        wxBitmap saveIcon("Textures/iconbar_save.png", wxBITMAP_TYPE_PNG);
        wxBitmap saveallIcon("Textures/iconbar_saveall.png", wxBITMAP_TYPE_PNG);
        wxToolBar* toolbar = CreateToolBar();
        toolbar->AddTool(ID_New, "New", newIcon, "Create a new mod");
        toolbar->AddTool(ID_Open, "Open", openIcon, "Open an existing mod");
        toolbar->AddTool(ID_Save, "Save", saveIcon, "Save the current tab");
        toolbar->AddTool(ID_SaveAll, "Save All", saveallIcon, "Save all active tabs");
        toolbar->Realize();
    }

    void MainFrame::InitStatusbar() {
        CreateStatusBar();
        SetStatusText("");
    }

    void MainFrame::InitTreectrl() {
        treectrl = new wxTreeCtrl(this, ID_MainTree, wxDefaultPosition, wxSize(250, 200), wxTR_DEFAULT_STYLE | wxTR_EDIT_LABELS);
        treectrl->Bind(wxEVT_TREE_ITEM_ACTIVATED, &MainFrame::OnTreeClick, this, ID_MainTree);
        treectrl->Bind(wxEVT_TREE_ITEM_MENU, &MainFrame::OnTreeMenu, this, ID_MainTree);
        treectrl->Bind(wxEVT_TREE_BEGIN_LABEL_EDIT, &MainFrame::OnTreeBeginLabelEdit, this, ID_MainTree);
        treectrl->Bind(wxEVT_TREE_END_LABEL_EDIT, &MainFrame::OnTreeEndLabelEdit, this, ID_MainTree);
        treectrl->Bind(wxEVT_MOUSEWHEEL, &MainFrame::OnMousewheel, this);
    }

    void MainFrame::InitNotebook() {
        notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_CLOSE_ON_ALL_TABS);
        notebook->Bind(wxEVT_MOUSEWHEEL, &MainFrame::OnMousewheel, this);
        notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &MainFrame::OnNotebookPageClose, this);
        notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &MainFrame::OnNotebookPageChanged, this);
    }

    void MainFrame::InitSizer() {
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        sizer->Add(notebook, 1, wxEXPAND);
        sizer->Add(treectrl, 0, wxEXPAND);
        SetSizerAndFit(sizer);
    }

    void MainFrame::GenTreeCtrl(wxTreeItemId& id, TreeMap& tm) {
        for (auto& [name, leaf] : tm.leaves) {
            auto node = treectrl->AppendItem(id, name, -1, -1, new wxMainTreeData(move(leaf.node)));
            GenTreeCtrl(node, leaf);
        }
    }

    void MainFrame::RefreshTreectrl() {
        treectrl->Freeze();
        treectrl->DeleteAllItems();
        TreeMap tm;
        for (auto& model : wxGetApp().GetModel()) {
            tm.merge(model->GetTreeMap());
        }
        auto root = treectrl->AddRoot(wxGetApp().GetModName(),-1,-1, new wxMainTreeData(move(tm.node)));
        GenTreeCtrl(root, tm);
        treectrl->Expand(root);
        treectrl->Thaw();
    }

    void MainFrame::OnMousewheel(wxMouseEvent& event) {
        if (notebook->IsMouseInWindow()) {
            if (notebook->GetCurrentPage()) {
                wxWindow* page = notebook->GetCurrentPage();
                bool foundChild = true;
                while (foundChild) {
                    foundChild = false;
                    for (wxWindow* child : page->GetChildren()) {
                        if (child->IsMouseInWindow()) {
                            page = child;
                            foundChild = true;
                            break;
                        }
                    }
                }
                if (page->IsMouseInWindow()) {
                    wxPostEvent(page->GetEventHandler(), event);
                }
            }
        }
        else if (treectrl->IsMouseInWindow()) {
            treectrl->ScrollLines(3 * (-event.GetWheelRotation() / event.GetWheelDelta()));
        }
    }

    void MainFrame::OnMenuExit(wxCommandEvent& event) {
        Close(true);
    }

    void MainFrame::OnMenuAbout(wxCommandEvent& event) {
        wxMessageBox("AMT is designed for personal use only.\nIcons courtesy of https://icons8.com.", "About AMT", wxOK | wxICON_INFORMATION);
    }

    void MainFrame::OnMenuSave(wxCommandEvent& event) {
        if (notebook->GetCurrentPage()) {
            IPanel* page = dynamic_cast<IPanel*>(notebook->GetCurrentPage());
            if (!page->IsSaved()) {
                page->HandleSave();
            }
        }
        wxGetApp().SaveEnviroment();
    }

    void MainFrame::OnMenuSaveAll(wxCommandEvent& event) {
        for (size_t i = 0U; i < notebook->GetPageCount(); i++) {
            IPanel* page = dynamic_cast<IPanel*>(notebook->GetPage(i));
            if (!page->IsSaved()) {
                page->HandleSave();
            }
        }
        wxGetApp().SaveEnviroment();
    }

    void MainFrame::OnTreeClick(wxTreeEvent& event) {
        wxMainTreeData* itemData = static_cast<wxMainTreeData*>(treectrl->GetItemData(event.GetItem()));
        if (itemData == NULL) {
            treectrl->Toggle(event.GetItem());
        }
        else if (activeTab.count(itemData)) {
            notebook->ChangeSelection(notebook->GetPageIndex(activeTab[itemData]));
        }
        else if(itemData->CanDisplay()) {
            IPanel* panel = itemData->CreatePanel();
            notebook->AddPage(panel, panel->GetTabName(), true);
            panel->InitPanelEvents();
            activeTab.insert_or_assign(itemData, panel);
        }
        else {
            treectrl->Toggle(event.GetItem());
        }
    }

    void MainFrame::OnTreeMenu(wxTreeEvent& event) {
        treectrl->SelectItem(event.GetItem());
        wxMainTreeData* itemData = static_cast<wxMainTreeData*>(treectrl->GetItemData(event.GetItem()));
        wxMenu menu;
        if (itemData != NULL) {
            if (itemData->CanDisplay()) {
                menu.Append(ID_MainTreeMenuDisplay, "Open");
                menu.Bind(wxEVT_MENU, &MainFrame::OnTreeMenuDisplay, this, ID_MainTreeMenuDisplay, -1,event.Clone());
            }
            if (itemData->CanAddLeaf()) {
                menu.Append(ID_MainTreeMenuAddLeaf, "Add...");
                menu.Bind(wxEVT_MENU, &MainFrame::OnTreeMenuAddLeaf, this, ID_MainTreeMenuAddLeaf, -1, event.Clone());
            }
            if (itemData->CanRename()) {
                menu.Append(ID_MainTreeMenuRename, "Rename");
                menu.Bind(wxEVT_MENU, &MainFrame::OnTreeMenuRename, this, ID_MainTreeMenuRename, -1, event.Clone());
            }
            if (itemData->CanDelete()) {
                menu.Append(ID_MainTreeMenuDelete, "Delete");
                menu.Bind(wxEVT_MENU, &MainFrame::OnTreeMenuDelete, this, ID_MainTreeMenuDelete, -1, event.Clone());
            }
        }
        menu.Append(ID_MainTreeMenuExpand, "Expand All");
        menu.Bind(wxEVT_MENU, &MainFrame::OnTreeMenuExpand, this, ID_MainTreeMenuExpand);
        menu.Append(ID_MainTreeMenuCollapse, "Collapse All");
        menu.Bind(wxEVT_MENU, &MainFrame::OnTreeMenuCollapse, this, ID_MainTreeMenuCollapse);
        PopupMenu(&menu);
    }

    void MainFrame::OnTreeMenuDisplay(wxCommandEvent& event) {
        try {
            OnTreeClick(*static_cast<wxTreeEvent*>(event.GetEventUserData()));
        }
        catch (InterfaceException& e) {
            wxMessageBox(e.what(), "Error", wxICON_ERROR | wxOK);
        }
    }

    void MainFrame::OnTreeMenuAddLeaf(wxCommandEvent& event) {
        wxTreeItemId id = static_cast<wxTreeEvent*>(event.GetEventUserData())->GetItem();
        std::string name = treectrl->GetItemText(id).ToStdString();
        wxMainTreeData* itemData = static_cast<wxMainTreeData*>(treectrl->GetItemData(id));
        if (itemData != NULL) {
            wxTextEntryDialog dialog(this, "Please enter the name for the new data", "Adding new leaf to " + name, "", wxCAPTION | wxCLOSE_BOX | wxSTAY_ON_TOP | wxOK | wxCANCEL);
            wxTextValidator valid{ wxFILTER_EXCLUDE_CHAR_LIST };
            valid.AddCharExcludes(invalidChar);
            dialog.SetValidator(valid);
            while (dialog.ShowModal() == wxID_OK) {
                try{
                    treectrl->AppendItem(id, dialog.GetValue(),-1,-1, new wxMainTreeData(itemData->CreateLeaf(dialog.GetValue().ToStdString())));
                    treectrl->SortChildren(id);
                    break;
                }
                catch(InterfaceException& e) {
                    wxMessageBox(e.what(), "Error", wxOK | wxICON_ERROR);
                }
            }
        }
    }

    void MainFrame::OnTreeMenuRename(wxCommandEvent& event) {
        wxTreeItemId id = static_cast<wxTreeEvent*>(event.GetEventUserData())->GetItem();
        wxTextCtrl* textctrl = treectrl->EditLabel(id);
        wxTextValidator valid{ wxFILTER_EXCLUDE_CHAR_LIST };
        valid.AddCharExcludes(invalidChar);
        textctrl->SetValidator(valid);
    }

    void MainFrame::OnTreeBeginLabelEdit(wxTreeEvent& event) {
        wxMainTreeData* itemData = static_cast<wxMainTreeData*>(treectrl->GetItemData(event.GetItem()));
        if (itemData == NULL) {
            event.Veto();
        }
        else if (!itemData->CanRename()) {
            event.Veto();
        }
    }

    void MainFrame::OnTreeEndLabelEdit(wxTreeEvent& event) {
        string name = event.GetLabel().ToStdString();
        wxMainTreeData* itemData = static_cast<wxMainTreeData*>(treectrl->GetItemData(event.GetItem()));
        if (itemData == NULL) {
            event.Veto();
        }
        else if (!itemData->CanRename()) {
            event.Veto();
        }
        else {
            try {
                itemData->Rename(event.GetLabel().ToStdString());
                treectrl->SetItemText(event.GetItem(), event.GetLabel());
                if (activeTab.count(itemData)) {
                    for (size_t i = 0; i < notebook->GetPageCount(); i++) {
                        if (notebook->GetPage(i) == activeTab.at(itemData)) {
                            notebook->SetPageText(i, activeTab.at(itemData)->GetTabName());
                            activeTab.at(itemData)->SetEdited();
                            break;
                        }
                    }
                }
                treectrl->SortChildren(treectrl->GetItemParent(event.GetItem()));
            }
            catch (InterfaceException& e) {
                wxMessageBox(e.what(), "Error", wxOK | wxICON_ERROR);
                event.Veto();
            }
        }
    }

    void MainFrame::OnTreeMenuDelete(wxCommandEvent& event) {
        wxTreeItemId id = static_cast<wxTreeEvent*>(event.GetEventUserData())->GetItem();
        wxMainTreeData* itemData = static_cast<wxMainTreeData*>(treectrl->GetItemData(id));
        wxMessageDialog dialog(this, "Are you sure you wish to delete \"" + treectrl->GetItemText(id) + "\"?\nThis action cannot be undone.", "Deleting: " + treectrl->GetItemText(id), wxOK | wxCANCEL | wxICON_QUESTION);
        if (dialog.ShowModal() == wxID_OK) {
            try {
                if (activeTab.count(itemData)) {
                    for (size_t i = 0; i < notebook->GetPageCount(); i++) {
                        if (notebook->GetPage(i) == activeTab.at(itemData)) {
                            notebook->DeletePage(i);
                            break;
                        }
                    }
                    activeTab.erase(itemData);
                }
                itemData->Delete();
                treectrl->Delete(id);
            }
            catch (InterfaceException& e) {
                wxMessageBox(e.what(), "Error", wxOK | wxICON_ERROR);
            }
        }
    }

    void MainFrame::OnTreeMenuExpand(wxCommandEvent& event) {
        treectrl->ExpandAll();
    }

    void MainFrame::OnTreeMenuCollapse(wxCommandEvent& event) {
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
    
    void MainFrame::OnNotebookPageClose(wxAuiNotebookEvent& event) {
        auto removeActiveTab = [this](IPanel* page) { //Replace with erase_if in c++20
            for (auto i = activeTab.begin(), last = activeTab.end(); i != last; ) {
                if (i->second == page) {
                    i = activeTab.erase(i);
                }
                else {
                    ++i;
                }
            }
        };
        if (event.GetSelection() != wxNOT_FOUND) {
            IPanel* page = static_cast<IPanel*>(notebook->GetPage(event.GetSelection()));
            if (!page->IsSaved()) {
                wxMessageDialog diag{ this, "Do you wish to save changes?", "Closing : " + notebook->GetCurrentPage()->GetLabel(), wxYES_NO | wxCANCEL | wxICON_QUESTION | wxCENTRE | wxSTAY_ON_TOP };
                int selection = diag.ShowModal();
                if (selection == wxID_YES) {
                    page->HandleSave();
                    wxGetApp().SaveEnviroment();
                    removeActiveTab(page);
                    event.Skip();
                }
                else if (selection == wxID_NO) {
                    removeActiveTab(page);
                    event.Skip();
                }
                else {
                    event.Veto();
                }
            }
            else {
                removeActiveTab(page);
                event.Skip();
            }
        }
        else {
            event.Veto();
        }
    }

    void MainFrame::OnNotebookPageChanged(wxAuiNotebookEvent& event) {
        if (!(notebook->GetPage(event.GetSelection()))->HasCapture()) {
            notebook->ChangeSelection(event.GetSelection()); //Forces event capture when changed to unused tabs
        }
        event.Skip();
    }

    void MainFrame::OnPlaceholder(wxCommandEvent& event) {
        wxLogMessage("Hello world");
    }

    //IPANEL---------------------------------------------------------------------------------------------------------------------------------------------

    IPanel::IPanel() : wxPanel{ wxGetApp().GetMainFrame() } {
        Bind(wxEVT_MOUSEWHEEL, &IPanel::OnMousewheel, this);
    }

    void IPanel::InitPanelEvents() {
        GenPanelEvents(this->GetChildren());
    }

    void IPanel::GenPanelEvents(wxWindowList& winlist) {
        for (wxWindow* child : winlist) {
            child->Bind(wxEVT_MOUSEWHEEL, &IPanel::OnChildMousewheel, this, wxID_ANY, wxID_ANY, new wxWindowObject(child));
            GenPanelEvents(child->GetChildren());
        }
    }

    void IPanel::SetEdited() {
        if (saved) {
            auto nb = wxGetApp().GetMainFrame()->GetNotebook();
            auto id = nb->GetPageIndex(this);
            auto label = nb->GetPageText(id);
            label.Append("*");
            nb->SetPageText(id, label);
            saved = false;
        }
    }

    void IPanel::HandleSave() {
        if (!saved) {
            UpdateData();
            auto nb = wxGetApp().GetMainFrame()->GetNotebook();
            auto id = nb->GetPageIndex(this);
            auto label = nb->GetPageText(id);
            label.RemoveLast();
            nb->SetPageText(id, label);
            saved = true;
        }
    }

    void IPanel::OnMousewheel(wxMouseEvent& event) {
        if (!IsMouseInWindow()) {
            wxPostEvent(GetParent()->GetEventHandler(), event);
        }
        else {
            for (wxWindow* child : this->GetChildren()) {
                if (event.GetWheelAxis() == wxMouseWheelAxis::wxMOUSE_WHEEL_VERTICAL) {
                    if (child->IsMouseInWindow()) {
                        child->ScrollLines(3 * (-event.GetWheelRotation() / event.GetWheelDelta()));
                        return;
                    }
                }
                else if (event.GetWheelAxis() == wxMouseWheelAxis::wxMOUSE_WHEEL_HORIZONTAL) {
                    if (child->IsMouseInWindow()) {
                        if (child->HasScrollbar(wxHORIZONTAL)) {
                            child->ScrollWindow(50 * (-event.GetWheelRotation() / event.GetWheelDelta()), 0);
                        }
                        return;
                    }
                }
            }
            event.Skip();
        }
    }

    void IPanel::OnChildMousewheel(wxMouseEvent& event) {
        wxPostEvent(this->GetEventHandler(), event);
    }

    //COMMON--------------------------------------------------------------------------------------------------------------------------------------------

    BindableFolder IStringParser::Parse(const Container<StringData>& data) {
        BindableFolder ctnr;
        StrParse(data, ctnr);
        return ctnr;
    }

    void IStringParser::StrParse(const Container<StringData>& data, BindableFolder& ctnr) {
        if (data.IsBindable()) {
            ctnr.SetBindPrefix(data.GetBindPrefix());
        }
        for (auto& file : data) {
            auto& str = file.GetString();
            BinaryFile bf(str.length(), file.GetName());
            copy(str.cbegin(), str.cend(), bf.begin());
            ctnr.insert(move(bf));
        }
    }

    Container<StringData> IStringParser::Parse(BindableFolder&& data) {
        Container<StringData> ctnr;
        StrParse(data, ctnr);
        return ctnr;
    }

    void IStringParser::StrParse(BindableFolder& data, Container<StringData>& ctnr) {
        if (data.IsBindable()) {
            ctnr.SetBindPrefix(data.GetBindPrefix());
        }
        for (auto& file : data) {
            ctnr.Insert(StringData(file.GetFilename() + file.GetExtension(), string(file.begin(), file.end())));
        }
    }

    StringPanel::StringPanel(const StringData& data) : IPanel{}, ref{ data } {
        InitTextctrl();
        InitSizer();
    }

    void StringPanel::InitTextctrl() {
        textctrl = new wxTextCtrl(this, ID_StringTextctrl, ref.GetString(), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxHSCROLL | wxTE_RICH);
        textctrl->Bind(wxEVT_TEXT, &StringPanel::OnTextEdit, this, ID_StringTextctrl);
    }

    void StringPanel::InitSizer() {
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        sizer->Add(textctrl, 1, wxEXPAND);
        SetSizerAndFit(sizer);
    }

    void StringPanel::OnTextEdit(wxCommandEvent& event) {
        SetEdited();
        event.Skip();
    }

    XmlPanel::XmlPanel(const StringData& data) : IPanel{}, ref{ data } {
        InitTextctrl();
        InitSizer();
    }

    void XmlPanel::InitTextctrl() {
        textctrl = new wxStyledTextCtrl(this, ID_XmlTextctrl);
        textctrl->AddText(ref.GetString());
        textctrl->SetLexer(wxSTC_LEX_XML);
        textctrl->StyleSetForeground(wxSTC_H_TAG, wxColor(0, 0, 255));
        textctrl->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColor(138, 43, 226));
        textctrl->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColor(255, 60, 10));
        textctrl->StyleSetForeground(wxSTC_H_XMLSTART, wxColor(255, 60, 10));
        textctrl->StyleSetForeground(wxSTC_H_XMLEND, wxColor(255, 60, 10));
        textctrl->SetUseTabs(false);
        textctrl->SetTabWidth(4);
        textctrl->SetIndentationGuides(wxSTC_IV_LOOKFORWARD);
        textctrl->StyleSetFont(wxSTC_STYLE_DEFAULT, wxFont(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE)));
        textctrl->SetMarginWidth(0, 50);
        textctrl->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(90, 90, 90));
        textctrl->StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(230, 230, 230));
        textctrl->SetMarginType(0, wxSTC_MARGIN_NUMBER);
        textctrl->Bind(wxEVT_STC_MODIFIED, &XmlPanel::OnTextEdit, this, ID_XmlTextctrl);
    }

    void XmlPanel::InitSizer() {
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        sizer->Add(textctrl, 1, wxEXPAND);
        SetSizerAndFit(sizer);
    }

    void XmlPanel::UpdateData() { 
        ref.SetString(textctrl->GetText().ToStdString()); 
    }

    void XmlPanel::OnTextEdit(wxStyledTextEvent& event) {
        if (event.GetModificationType() & (wxSTC_MOD_INSERTTEXT | wxSTC_MOD_DELETETEXT))
            SetEdited();
    }
}

//Starts the wxWidgets "main function"
wxIMPLEMENT_APP(amt::AmtApp);