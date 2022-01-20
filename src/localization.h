#pragma once

#include "amt.h"
#include <codecvt>
#include <sstream>
#include <set>
#include <vector>
#include <string.h>
#include <wx/treectrl.h>
#include <wx/grid.h>

namespace Localization {
	class Translation : public wxTreeItemData {
	public:
		inline static const std::vector<std::wstring> LANG_NAME{ L"English",L"Dutch",L"German" ,L"French" ,L"Spanish" ,L"Italian" ,L"Brazilian-Portuguese" ,L"Russian" ,L"Simplified-Chinese" };
		Translation() : wxTreeItemData{}, lang { LANG_NAME.size() } {};
		Translation(const std::wstring& id) : wxTreeItemData{}, id{ id }, lang{ LANG_NAME.size() } {}
		Translation(std::wstring&& id) : wxTreeItemData{}, id{ id }, lang{ LANG_NAME.size() } {}
		std::wstring id;
		std::vector<std::wstring> lang;
		friend bool operator==(const Translation& lhs, const Translation& rhs) { return _wcsicmp(lhs.id.c_str(), rhs.id.c_str()) == 0; }
		friend bool operator!=(const Translation& lhs, const Translation& rhs) { return !operator==(lhs, rhs); }
		friend bool operator< (const Translation& lhs, const Translation& rhs) { return _wcsicmp(lhs.id.c_str(), rhs.id.c_str()) < 0; }
		friend bool operator> (const Translation& lhs, const Translation& rhs) { return  operator< (rhs, lhs); }
		friend bool operator<=(const Translation& lhs, const Translation& rhs) { return !operator> (lhs, rhs); }
		friend bool operator>=(const Translation& lhs, const Translation& rhs) { return !operator< (lhs, rhs); }
	};

	class Data : public amt::IData {
	public:
		Data() = default;
		Data(const std::string& key, const std::string& tabPrefix, std::vector<std::wstring> prefixes = {}) : amt::IData{ key }, tabPrefix{ tabPrefix }, prefixes{ prefixes } {}
		std::set<Translation> translations;
		std::vector<std::wstring> prefixes;
		std::string tabPrefix;
		unsigned int FindPrefix(const std::wstring& str);
		inline static const std::wstring misc{ L"<MISC>" };
	};

	class LParser : public amt::IParser<Data> {
	public:
		LParser(std::string filename, bool splitParse = false, bool treeDisplay = false)
			: amt::IParser<Data>{ std::filesystem::path("Data") / std::filesystem::path("Localization"), amt::ParseMode::DEFAULT, [filename](const std::filesystem::path& fp) { if (fp.has_stem()) return fp.stem().string() == filename; else return false; } }, filename{ filename }, splitParse{ splitParse }, treeDisplay{ treeDisplay } {}
		virtual apcl::BindableFolder Parse(const amt::Container<Data>& data) override;
		virtual amt::Container<Data> Parse(apcl::BindableFolder&& data) override;
	private:
		inline static const std::vector<std::wstring> ALL_PREFIX{ L"AccountUnlock",L"Achievement",L"Announcer",L"Armory",L"Awesomenauts",L"Awesomepoints",L"Background",L"Beta",L"Brawl",L"Briefing",L"Button",L"BuyButton",L"CancelMatchmaking",L"CharacterSelect",L"CharacterStat",L"CharUnlock",L"Chat",L"Controls",L"Currency",L"CustomGame",L"DailyMission",L"Demo",L"DLC",L"Downloadable_Content",L"Droid",L"Dropdown",L"Droppod",L"EditorGUI",L"Enable",L"Experience_Gain",L"ExplodeOnDeath",L"FirstWin",L"FriendStatus",L"Funnel",L"Game",L"Hint",L"Invite",L"Joystick",L"KC",L"Keyboard",L"Kill",L"LandingAreaScreen",L"Language",L"League",L"Level",L"LiveContent",L"LiveTile",L"M_Abandon",L"M_Armory",L"M_Banner",L"M_BTN",L"M_CharacterSelect",L"M_Community",L"M_CustomGame",L"M_DLC",L"M_Fanart",L"M_Filter",L"M_FirstPlay",L"M_HelpAndOptions",L"M_IngameMenu",L"M_Leaderboards",L"M_NewPatchNotice",L"M_Party",L"M_Popup",L"M_Portrait",L"M_Profile",L"M_Progress",L"M_Recruitment",L"M_Safeframe",L"M_Scoreboard",L"M_Shop",L"M_WhatsNew",L"MainMenu",L"Match",L"MaxHealth",L"MaxPurchasableItems",L"Menu_Watch",L"MenuArmory",L"MenuCustomMatchBrowser",L"MenuModding",L"MenuOptions",L"MenuReplay",L"Metagame",L"Modding",L"Month_Short_Form",L"NameDrop",L"NameIcon",L"Navigation",L"Network",L"NewGameFound",L"NoReplays",L"Notification",L"OptionsMenu",L"OverrideUpgradeGoldCost",L"Party",L"Passive",L"Passport",L"PlayAnotherGame",L"Player",L"Popup",L"Profile",L"PromotionalText",L"PublishingMod",L"PurchaseWindow",L"Rejoin",L"Replay",L"Report",L"Respawn",L"RichPresence",L"RM",L"RP",L"Scoreboard",L"SearchMatchID", L"Share",L"Shop",L"Skin",L"SolarBossOnDeath",L"Starting",L"StatusEffect",L"Tab",L"Team",L"TLTP",L"Tooltip",L"Transaction",L"TurnIncognito",L"Tut_IG",L"Tut_Intro",L"Tut_SC2",L"Tutorial",L"UI",L"Unlock",L"Upgrade",L"UserReport",L"Waiting",L"Watch",L"WindowMode",L"WinWhenKill",L"Workshop",L"XP",L"You" };
		inline static const std::vector<std::wstring> HERO_PREFIX{ L"M_DLCItem_Skin",L"M_Popup",L"M_PortraitDetails",L"NameChar",L"Playstyle",L"PortraitCharacter",L"SkinPack",L"TekstBio",L"TekstDifficulty",L"UI_Skillbutton",L"Upgrade"};
		bool splitParse;
		bool treeDisplay;
		std::string filename;
	};

	class Parser : public LParser { public: Parser() : LParser{ "Localization", true, true } {} };
	class ParserAUTO : public LParser { public: ParserAUTO() : LParser{ "LocalizationAUTO", true } {} };
	class ParserIDS : public LParser { public: ParserIDS() : LParser{ "LocalizationIDS"} {} };
	class ParserPC : public LParser { public: ParserPC() : LParser{ "LocalizationPC"} {} };

	class Model : public amt::IModel {
	public:
		Model() : dm{ this }, dmAUTO{ this }, dmIDS{ this }, dmPC{ this } {}
		virtual std::string GetLoadingMessage() const override { return "Loading Localizations..."; }
		virtual amt::TreeMap GetTreeMap() override;
	private:
		amt::DataManager<Data, Parser> dm;
		amt::DataManager<Data, ParserAUTO> dmAUTO;
		amt::DataManager<Data, ParserIDS> dmIDS;
		amt::DataManager<Data, ParserPC> dmPC;
	};

	class Panel : public amt::IPanel {
	public:
		Panel() = delete;
		Panel(Data& data, bool hasPartData = false, bool hasHeroPart = false);
		virtual std::string GetTabName() const override { return ref.tabPrefix + ref.GetName(); }
		virtual void UpdateData() override;
	private:
		Data& ref;
		wxTreeCtrl* treectrl;
		wxGrid* grid;
		std::set<Translation*> tlList;
		Translation* activeTl{ nullptr };
		bool hasPartData{ false };
		bool hasHeroPart{ false };
		void InitTreectrl();
		void InitGrid();
		void InitSizer();
		void FormatGrid();
		void OnResize(wxSizeEvent& event);
		void OnTreeClick(wxTreeEvent& event);
		void OnTreeItemMenu(wxTreeEvent& event);
		void OnTreeMenuNew(wxCommandEvent& event);
		void OnTreeMenuRename(wxCommandEvent& event);
		void OnTreeMenuDelete(wxCommandEvent& event);
		void OnTreeMenuExpand(wxCommandEvent& event);
		void OnTreeMenuCollapse(wxCommandEvent& event);
		void OnTreeBeginLabelEdit(wxTreeEvent& event);
		void OnTreeEndLabelEdit(wxTreeEvent& event);
		void OnCellChanging(wxGridEvent& event);
		void OnCellChanged(wxGridEvent& event);
	};
}