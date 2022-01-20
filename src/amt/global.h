#pragma once

#include <vector>
#include <memory>
#include <filesystem>
#include <string>

namespace amt {
    class IModel;
    std::vector<std::unique_ptr<IModel>> GetModelList();
    static const std::string invalidChar{ "<>:\"/\\|?*@+=~" };//Characters that cannot be used in filenames (either by filesystem or game requirements)

    enum { //ID's for wx events
        ID_Placeholder = 1, ID_MainTree, ID_New, ID_Open, ID_Save, ID_SaveAll, ID_Export, ID_ExportAs,
        ID_MainTreeMenuExpand, ID_MainTreeMenuCollapse, ID_MainTreeMenuDisplay, ID_MainTreeMenuAddLeaf, ID_MainTreeMenuRename, ID_MainTreeMenuDelete,
        ID_StringTextctrl, ID_XmlTextctrl,
        ID_LocalizationTree, ID_LocalizationGrid,
        ID_LocalizationTreeMenuNew, ID_LocalizationTreeMenuRename, ID_LocalizationTreeMenuDelete, ID_LocalizationTreeMenuExpand, ID_LocalizationTreeMenuCollapse,
        ID_SettingsTree,
        ID_SettingsTreeMenuNew, ID_SettingsTreeMenuRename, ID_SettingsTreeMenuDelete, ID_SettingsTreeMenuAddProp, ID_SettingsTreeMenuExpand, ID_SettingsTreeMenuCollapse, 
        ID_SettingsPropMenuNew, ID_SettingsPropMenuRename, ID_SettingsPropMenuDelete, ID_SettingsPropMenuRemoveClass, ID_SettingsPropMenuExpand, ID_SettingsPropMenuCollapse
    };
}