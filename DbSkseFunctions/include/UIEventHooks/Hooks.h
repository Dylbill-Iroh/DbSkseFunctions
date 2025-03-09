#pragma once
//#include <Windows.h>
//#include <spdlog/sinks/basic_file_sink.h>
//#include <stdarg.h>
//#include <winbase.h>
//#include <iostream>
//#include <stdio.h>
//#include <chrono>
//#include <ctime>
//#include <algorithm>
//#include <string>
//#include "mini/ini.h"
#include "editorID.hpp"
#include "GeneralFunctions.h"
#include "STLThunk.h"

namespace logger = SKSE::log;

namespace UIEvents {
    bool isUnregistering_All = false;
    bool inventoryMenuHookInstalled = false;
    bool barterMenuHookInstalled = false;
    bool containerMenuHookInstalled = false;
    bool giftMenuHookInstalled = false;
    bool magicMenuHookInstalled = false;
    bool craftingMenuHookInstalled = false;
    bool favoritesMenuHookInstalled = false;
    std::vector<RE::BSFixedString> validSelectUserEventStrings;
    std::vector<RE::BSFixedString> validDropUserEventStrings;
    std::vector<RE::BSFixedString> validFavoriteUserEventStrings;
    RE::Actor* playerRef;
    RE::UI* ui;
    RE::UserEvents* userEvents;
    RE::BSFixedString uiEvent = "OnUiItemMenuEvent";

    std::vector<std::string> uiItemMenuNames = { 
        "inventorymenu",
        "bartermenu",
        "containermenu",
        "giftmenu",
        "magicmenu",
        "crafting menu",
        "favoritesmenu" };

    enum UIItemMenuEnum {
        UiItemMenuEnum_InventoryMenu,
        UiItemMenuEnum_BarterMenu,
        UiItemMenuEnum_ContainerMenu,
        UiItemMenuEnum_GiftMenu,
        UiItemMenuEnum_MagicMenu,
        UiItemMenuEnum_Crafting_Menu,
        UiItemMenuEnum_FavoritesMenu
    };

    enum UIEventEnumType {
        UiEventEnumType_SelectionChanged,
        UiEventEnumType_ItemSelected,
        UiEventEnumType_ItemDropped,
        UiEventEnumType_FavoredToggle,
        UiEventEnumType_start = UiEventEnumType_SelectionChanged,
        UiEventEnumType_end = UiEventEnumType_FavoredToggle
    };

    struct UIEventData {
        RE::VMHandle registeredHandle;
        std::string menuName;
        RE::TESForm* paramFilter;
        int eventType;

        //constructor
        UIEventData(RE::VMHandle akRegisteredHandle, std::string akMenuName, RE::TESForm* akParamFilter, int akEventType) :
            registeredHandle(akRegisteredHandle),
            menuName(akMenuName),
            paramFilter(akParamFilter),
            eventType(akEventType)
        {}
        
        inline bool operator==(UIEventData& b) {
            return (b.eventType == eventType
                && b.menuName == menuName
                && b.paramFilter == paramFilter
                && b.registeredHandle == registeredHandle);
        }
    };

    std::vector<UIEventData> registeredUIEventDatas;

    struct UiFormData {
        RE::TESForm* form;
        int count;
        bool selectedFromPlayerInventory;
        bool stolen;

        UiFormData(){
            form = nullptr;
            count = 0;
            selectedFromPlayerInventory = false;
            stolen = false;
            //logger::trace("default constructor");
        }

        UiFormData(RE::TESForm* akForm, int akCount, bool akSelectedFromPlayerInventory, bool akStolen) :
            form(akForm),
            count(akCount),
            selectedFromPlayerInventory(akSelectedFromPlayerInventory),
            stolen(akStolen)
        {
            //logger::trace("specific constuctor"); 
        }
            
        inline bool operator==(UiFormData& b)
        {
            return (b.form == form
                && b.selectedFromPlayerInventory == selectedFromPlayerInventory
                && b.count == count
                && b.stolen == stolen);
        }
    };

    int uiLastSelectedMsgEventType = 0;
    uint32_t lastUIEventRunTime = 0;
    uint32_t lastUIEventSelectRunTime = 0;
    UiFormData uiSelectedFormData;
    UiFormData uiLastSelectedFormData;
    std::string uiLastMenuEventName;

    bool UIEventDataMatchesParams(UIEventData& UIEventData, std::string& menuName, RE::TESForm* paramFilter, int& eventType) {
        if (UIEventData.menuName != "") {
            if (UIEventData.menuName != menuName) {
                return false;
            }
        }
        if (UIEventData.paramFilter) {
            if (UIEventData.paramFilter != paramFilter) {
                return false;
            }
        }
        if (UIEventData.eventType >= UiEventEnumType_start && UIEventData.eventType <= UiEventEnumType_end) {
            if (UIEventData.eventType != eventType) {
                return false;
            }
        }
        return true;
    }

    void SendUISelectionEvents(std::string menuName, UiFormData data, int eventType) {
        std::vector<RE::VMHandle> handles;
        for (int i = 0; i < registeredUIEventDatas.size(); i++) {
            auto& uiEventData = registeredUIEventDatas[i];
            if (UIEventDataMatchesParams(uiEventData, menuName, data.form, eventType)) {
                handles.push_back(uiEventData.registeredHandle);
            }
        }

        gfuncs::RemoveDuplicates(handles);
        auto* args = RE::MakeFunctionArguments((std::string)menuName, (RE::TESForm*)data.form, (int)eventType, (int)data.count, (bool)data.selectedFromPlayerInventory, (bool)data.stolen);
        gfuncs::SendEvents(handles, uiEvent, args);
    }

    bool shouldSkipSelectEvent(uint32_t& diff, int eventType) {
        if (diff < 50) {
            if (uiLastSelectedFormData == uiSelectedFormData && uiLastSelectedMsgEventType == eventType) {
                return true;
            }
        }
        return false;
    }

    template <class T>
    UiFormData GetUiSelectedFormData(T* menu, RE::UIMessage& message) {
        UiFormData akFormData;

        if (menu) {
            if (!IsBadReadPtr(menu, sizeof(menu))) {
                auto& runtimeData = menu->GetRuntimeData();
                //logger::trace("GetUiFormData: runtimeData found");

                if (runtimeData.itemList) {
                    if (!IsBadReadPtr(runtimeData.itemList, sizeof(runtimeData.itemList))) {
                        //logger::trace("GetUiFormData: itemList found");

                        RE::ItemList::Item* item = runtimeData.itemList->GetSelectedItem();
                        if (item) {
                            if (!IsBadReadPtr(item, sizeof(item))) {
                                //logger::trace("GetUiFormData: selected ItemList::Item found");

                                if (item->data.objDesc) {
                                    if (!IsBadReadPtr(item->data.objDesc, sizeof(item->data.objDesc))) {
                                        //logger::trace("GetUiFormData:  item->data.objDesc found");

                                        RE::TESForm* akForm = item->data.objDesc->object;
                                        if (gfuncs::IsFormValid(akForm)) {
                                            //logger::trace("GetUiFormData:  akForm[{}] found", gfuncs::GetFormName(akForm));

                                            int count = item->data.GetCount();
                                            //logger::trace("count is [{}]", count);

                                            RE::TESObjectREFR* owner = nullptr;

                                            if (item->data.owner) {
                                                RE::TESObjectREFRPtr refptr = nullptr;
                                                if (RE::TESObjectREFR::LookupByHandle(item->data.owner, refptr)) {
                                                    if (refptr) {
                                                        owner = refptr.get();
                                                    }
                                                }
                                            }

                                            bool selectFromPlayerInventory = (owner == playerRef);

                                            bool isStolen = false;
                                            RE::GFxValue gfxStolen;
                                            if (menu->uiMovie->GetVariable(&gfxStolen, "_root.Menu_mc.inventoryLists.itemList.selectedEntry.isStolen")) { //
                                                if (gfxStolen.IsBool()) {
                                                    isStolen = static_cast<bool>(gfxStolen.GetBool());
                                                    //logger::trace("selectedEntry.isStolen = [{}]", isStolen);
                                                }
                                            }

                                            //logger::trace("owner is [{}]", gfuncs::GetFormName(owner));

                                            return UiFormData(akForm, count, selectFromPlayerInventory, isStolen);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return akFormData;
    }

    UiFormData GetUiMagicSelectedFormData(RE::MagicMenu* menu, RE::UIMessage& message) {
        UiFormData data;

        if (menu) {
            if (!IsBadReadPtr(menu, sizeof(menu))) {
                RE::GFxValue value;
                if (menu->uiMovie->GetVariable(&value, "_root.Menu_mc.inventoryLists.itemList.selectedEntry.formId")) {
                    if (value.IsNumber()) {
                        RE::FormID formID = static_cast<std::uint32_t>(value.GetNumber());
                        RE::TESForm* akForm = RE::TESForm::LookupByID(formID);
                        if (gfuncs::IsFormValid(akForm)) {
                            return UiFormData(akForm, 1, true, false);
                        }
                    }
                }
            }
        }

        return data;
    }

    UiFormData GetUiCraftingSelectedFormData(RE::CraftingMenu* menu, RE::UIMessage& message) {
        UiFormData data;

        //auto* subMenu = menu->GetCraftingSubMenu(); //

        if (menu) {
            if (!IsBadReadPtr(menu, sizeof(menu))) {
                RE::GFxValue value;
                if (menu->uiMovie->GetVariable(&value, "_root.Menu.InventoryLists.panelContainer.itemList.selectedEntry.formId")) {
                    if (value.IsNumber()) {
                        RE::FormID formID = static_cast<std::uint32_t>(value.GetNumber());
                        RE::TESForm* akForm = RE::TESForm::LookupByID(formID);
                        if (gfuncs::IsFormValid(akForm)) {
                            int count = 1;
                            RE::GFxValue gfxCount;
                            if (menu->uiMovie->GetVariable(&gfxCount, "_root.Menu.InventoryLists.panelContainer.itemList.selectedEntry.count")) {
                                if (gfxCount.IsNumber()) {
                                    count = static_cast<int>(gfxCount.GetNumber());
                                    //logger::trace("selectedEntry.count = [{}]", count);
                                }
                            }
                            return UiFormData(akForm, count, false, false);
                        }
                    }
                }
            }
        }

        return data;
    }

    UiFormData GetUiFavortesSelectedFormData(RE::FavoritesMenu* menu, RE::UIMessage& message) {
        UiFormData data;

        //these are the variables I found for selectedEntry in the SkyUi source .As files.
        // I couldn't find where selectedEntry is actually defined
        //selectedEntry.raceName
        //selectedEntry.name
        //selectedEntry.text
        //selectedEntry.level
        //selectedEntry.index
        //selectedEntry.itemId
        //selectedEntry.formId
        //selectedEntry.enabled
        //selectedEntry.disabled
        //selectedEntry.flag
        //selectedEntry.count
        // 
        //selectedEntry.questTargetID
        //selectedEntry.completed
        //selectedEntry.instance
        //selectedEntry.active
        //selectedEntry.objectives
        //selectedEntry.type
        //selectedEntry.description
        //selectedEntry.stats

        //selectedEntry.filterFlag
        //selectedEntry.divider
        //selectedEntry.id
        //selectedEntry.value
        //selectedEntry.handleInput
        //selectedEntry.clipIndex
        //selectedEntry.textFormat
        //selectedEntry.handleInput
        //selectedEntry.chargeAdded
        //selectedEntry.itemIndex
        //selectedEntry._height
        //selectedEntry._width
        //selectedEntry.
        // 
        //MCM
        //selectedEntry.OPTION_EMPTY
        //selectedEntry.FLAG_DISABLED
        //selectedEntry.FLAG_HIDDEN
        //selectedEntry.FLAG_WITH_UNMAP
        //selectedEntry.

        if (menu) {
            if (!IsBadReadPtr(menu, sizeof(menu))) {
                RE::GFxValue value;
                if (menu->uiMovie->GetVariable(&value, "_root.MenuHolder.Menu_mc.itemList.selectedEntry.formId")) {
                    if (value.IsNumber()) {
                        RE::FormID formID = static_cast<std::uint32_t>(value.GetNumber());
                        RE::TESForm* akForm = RE::TESForm::LookupByID(formID);
                        if (gfuncs::IsFormValid(akForm)) {
                            int count = 0;

                            //this didn't work for the favorites menu
                            //RE::GFxValue gfxCount;
                            //if (menu->uiMovie->GetVariable(&gfxCount, "_root.MenuHolder.Menu_mc.itemList.selectedEntry.count")) {
                            //    if (gfxCount.IsNumber()) {
                            //        count = static_cast<int>(gfxCount.GetNumber());
                            //        logger::trace("selectedEntry.count = [{}]", count);
                            //    }
                            //}

                            RE::MagicItem* magicItem = akForm->As<RE::MagicItem>();
                            if (magicItem) {
                                count = 1;
                            }
                            else {
                                ////this didn't work reliably
                                //auto* container = playerRef->GetContainer();
                                //if (container) {
                                //    auto* boundObj = akForm->As<RE::TESBoundObject>();
                                //    if (boundObj) {
                                //        count = container->CountObjectsInContainer(boundObj);
                                //    }
                                //}

                                //if (count <= 0) { // CountObjectsInContainer doesn't work for all forms, for example (iron boots), getting item count from text instead. 
                                    std::string sText = "";
                                    RE::GFxValue text;
                                    if (menu->uiMovie->GetVariable(&text, "_root.MenuHolder.Menu_mc.itemList.selectedEntry.text")) {
                                        if (text.IsString()) {
                                            sText = static_cast<std::string>(text.GetString()); //this works and includes the number in the text, but it's easier to just get item count from player container.
                                        }
                                    }
                                    count = gfuncs::GetIntAfterCharInString(sText, '(', 1); //will return 1 if sText == "" or doesn't contain number.
                                    //logger::trace("sText = [{}] count = [{}]", sText, count);
                                //}
                            }
                            return UiFormData(akForm, count, true, false);
                        }
                    }
                }
            }
        }

        return data;
    }

    bool ShouldSkipUIItemEvent() {
        if (ui->closingAllMenus) {
            logger::trace("closingAllMenus = true");
            return true;
        }

        if (!ui->menuSystemVisible) {
            logger::trace("menuSystemVisible = false");
            return true;
        }

        if (!ui->IsItemMenuOpen()) {
            logger::trace("IsItemMenuOpen = false");
            return true;
        }
        return false;
    }

    void ProcessUiItemChangedEvent(UiFormData& data, std::string& menuName) {
        uiLastMenuEventName = menuName;

        if (uiSelectedFormData.form != data.form) {
            uiSelectedFormData = data;
            SendUISelectionEvents(menuName, data, UiEventEnumType_SelectionChanged);

            logger::debug("menu[{}] selected form changed to [{}] from player = [{}] count = [{}] stolen = [{}]",
                __func__, menuName, gfuncs::GetFormName(data.form), data.selectedFromPlayerInventory, data.count, data.stolen);
        }
    }

    void ProcessUiItemSelectEvent(RE::UIMessage& message, std::string& menuName) {
        uiLastMenuEventName = menuName;
        int type = (message.type.underlying());

        RE::BSUIMessageData* msgData = static_cast<RE::BSUIMessageData*>(message.data);
        //auto msgTypeString = gfuncs::GetBSUIMessageDataTypeString(msgData);
        
        uint32_t currentRunTime = RE::GetDurationOfApplicationRunTime();
        uint32_t diff = currentRunTime - lastUIEventSelectRunTime;

        if (diff < 40) {
            if (uiLastSelectedFormData == uiSelectedFormData ) {
                logger::trace("ui select skipped, runTimeDiff is {}", diff);
                //the select event can run twice for the same selected ui object depending on the button pressed to select. 
                //once for kUserEvent and once for kInventoryUpdate.
                //this will prevent the same selected ui object event from being sent to papyrus twice in a row.
                return;
            }
        }

        logger::trace("menu[{}] message type = [{}] uiSelectedForm = [{}] from player = [{}]",
            __func__, menuName, type, gfuncs::GetFormName(uiSelectedFormData.form), uiSelectedFormData.selectedFromPlayerInventory);

        lastUIEventSelectRunTime = currentRunTime;
        uiLastSelectedFormData = uiSelectedFormData;
        uiLastSelectedMsgEventType = type;

        if (msgData) {
            UiFormData selectedFormData = uiSelectedFormData;

            //leftEquip leftAttack

            if (selectedFormData.form) {
                if (gfuncs::GetIndexInVector(validSelectUserEventStrings, msgData->fixedStr) > -1 || msgData->fixedStr == "À¬") { //"À¬" for unrecognized (enter key)
                    if (msgData->fixedStr == userEvents->leftEquip || msgData->fixedStr == userEvents->leftAttack) {
                        if (menuName == "inventorymenu" || menuName == "favoritesmenu" || menuName == "containermenu" || menuName == "magicmenu") {
                            SendUISelectionEvents(menuName, selectedFormData, UiEventEnumType_ItemSelected);
                        }
                    }
                    else {
                        SendUISelectionEvents(menuName, selectedFormData, UiEventEnumType_ItemSelected);
                    }
                }
                else if (gfuncs::GetIndexInVector(validDropUserEventStrings, msgData->fixedStr) > -1) {
                    SendUISelectionEvents(menuName, selectedFormData, UiEventEnumType_ItemDropped);
                }
                else if (gfuncs::GetIndexInVector(validFavoriteUserEventStrings, msgData->fixedStr) > -1) {
                    SendUISelectionEvents(menuName, selectedFormData, UiEventEnumType_FavoredToggle);
                }
            }
        }
    }

    //processed from enter key press from InputEventSink in plugin.cpp
    void ProcessUiItemSelectEvent() {
        uint32_t currentRunTime = RE::GetDurationOfApplicationRunTime();
        uint32_t diff = currentRunTime - lastUIEventSelectRunTime;

        if (diff < 40) {
            logger::trace("ui select skipped, runTimeDiff is {}", diff);
            //the select event can run twice for the same selected ui object depending on the button pressed to select. 
            //once for kUserEvent and once for kInventoryUpdate.
            //this will prevent the same selected ui object event from being sent to papyrus twice in a row.
            return;
        }

        logger::trace("menu[{}] message type = [{}] uiSelectedForm = [{}] from player = [{}]",
            __func__, uiLastMenuEventName, 7, gfuncs::GetFormName(uiSelectedFormData.form), uiSelectedFormData.selectedFromPlayerInventory);

        lastUIEventSelectRunTime = currentRunTime;
        uiLastSelectedFormData = uiSelectedFormData;
        uiLastSelectedMsgEventType = 7;

        UiFormData selectedFormData = uiSelectedFormData;

        if (selectedFormData.form) {
            SendUISelectionEvents(uiLastMenuEventName, selectedFormData, UiEventEnumType_ItemSelected);
        }
    }

    template <class T>
    void ProcessUiItemMenuEvent(T* menu, RE::UIMessage& message, std::string menuName) {
        if (ShouldSkipUIItemEvent()) {
            return;
        }

        if (message.type == RE::UI_MESSAGE_TYPE::kScaleformEvent || message.type == RE::UI_MESSAGE_TYPE::kShow || message.type == RE::UI_MESSAGE_TYPE::kReshow) {
            UiFormData data = GetUiSelectedFormData(menu, message);
            ProcessUiItemChangedEvent(data, menuName);
        }
        else if (message.type == RE::UI_MESSAGE_TYPE::kUserEvent || message.type == RE::UI_MESSAGE_TYPE::kInventoryUpdate) {
            ProcessUiItemSelectEvent(message, menuName);
        }
        else if (message.type == RE::UI_MESSAGE_TYPE::kHide || message.type == RE::UI_MESSAGE_TYPE::kForceHide) {
            UiFormData akData;
            uiSelectedFormData = akData;
        }
    }

    void ProcessUiMagicMenuEvent(RE::MagicMenu* menu, RE::UIMessage& message, std::string menuName) {
        if (ShouldSkipUIItemEvent()) {
            return;
        }

        if (message.type == RE::UI_MESSAGE_TYPE::kScaleformEvent || message.type == RE::UI_MESSAGE_TYPE::kShow || message.type == RE::UI_MESSAGE_TYPE::kReshow) {
            UiFormData data = GetUiMagicSelectedFormData(menu, message);
            ProcessUiItemChangedEvent(data, menuName);
        }
        else if (message.type == RE::UI_MESSAGE_TYPE::kUserEvent || message.type == RE::UI_MESSAGE_TYPE::kInventoryUpdate) {
            ProcessUiItemSelectEvent(message, menuName);
        }
        else if (message.type == RE::UI_MESSAGE_TYPE::kHide || message.type == RE::UI_MESSAGE_TYPE::kForceHide) {
            UiFormData akData;
            uiSelectedFormData = akData;
        }
    }

    void ProcessUiCraftingMenuEvent(RE::CraftingMenu* menu, RE::UIMessage& message, std::string menuName) {
        if (ShouldSkipUIItemEvent()) {
            return;
        }

        //logger::trace("message.type = [{}]", message.type.underlying());

        if (message.type == RE::UI_MESSAGE_TYPE::kScaleformEvent || message.type == RE::UI_MESSAGE_TYPE::kShow || message.type == RE::UI_MESSAGE_TYPE::kReshow) {
            UiFormData data = GetUiCraftingSelectedFormData(menu, message);
            ProcessUiItemChangedEvent(data, menuName);
        }
        else if (message.type == RE::UI_MESSAGE_TYPE::kUserEvent || message.type == RE::UI_MESSAGE_TYPE::kInventoryUpdate) {
            //bool isEnabled = false;
            //RE::GFxValue gfxEnabled;
            //if (menu->uiMovie->GetVariable(&gfxEnabled, "_root.Menu.InventoryLists.panelContainer.itemList.selectedEntry.enabled")) { //
            //    //logger::trace("got selectedEntry.enabled variable");

            //    if (gfxEnabled.IsBool()) {
            //        isEnabled = static_cast<bool>(gfxEnabled.GetBool());
            //        //logger::trace("selectedEntry.enabled = [{}]", isEnabled);
            //    }
            //}

            //logger::trace("messageBox open = [{}]", ui->IsMenuOpen(RE::MessageBoxMenu::MENU_NAME));

            //RE::BSUIMessageData* msgData = static_cast<RE::BSUIMessageData*>(message.data);
            //auto msgTypeString = gfuncs::GetBSUIMessageDataTypeString(msgData);
            //logger::trace("msgTypeString = [{}]", msgTypeString);

            //if (msgTypeString == "Unrecognized") {
            //    logger::trace("Unrecognized string = [{}]", msgData->fixedStr);
            //}

            //if (isEnabled) {
                ProcessUiItemSelectEvent(message, menuName);
            //}
        }
        else if (message.type == RE::UI_MESSAGE_TYPE::kHide || message.type == RE::UI_MESSAGE_TYPE::kForceHide) {
            UiFormData akData;
            uiSelectedFormData = akData;
        }
    }
    
    void ProcessUiFavoritesMenuEvent(RE::FavoritesMenu* menu, RE::UIMessage& message, std::string menuName) {
        if (ShouldSkipUIItemEvent()) {
            return;
        }
        if (message.type == RE::UI_MESSAGE_TYPE::kScaleformEvent || message.type == RE::UI_MESSAGE_TYPE::kShow || message.type == RE::UI_MESSAGE_TYPE::kReshow) {
            UiFormData data = GetUiFavortesSelectedFormData(menu, message);
            ProcessUiItemChangedEvent(data, menuName);
        }
        else if (message.type == RE::UI_MESSAGE_TYPE::kUserEvent || message.type == RE::UI_MESSAGE_TYPE::kInventoryUpdate) {
            ProcessUiItemSelectEvent(message, menuName);
        }
        else if (message.type == RE::UI_MESSAGE_TYPE::kHide || message.type == RE::UI_MESSAGE_TYPE::kForceHide) {
            UiFormData akData;
            uiSelectedFormData = akData;
        }
    }

    struct InventoryMenuHook {
    
        inline static constexpr auto VTABLE = RE::VTABLE_InventoryMenu;

        static RE::UI_MESSAGE_RESULTS thunk(RE::IMenu* menu, RE::UIMessage& message) {
            uint32_t appRunTime = RE::GetDurationOfApplicationRunTime();
            auto result = func(menu, message);

            //logger::trace("inventory event: appRunTime[{}] lastUIEventRunTime[{}]", appRunTime, lastUIEventRunTime);

            //this inventory event is triggered multiple times when opening or closing the menu. 
            //if it already ran then skip
            if (lastUIEventRunTime == appRunTime) {
                //logger::trace("inventory event: event already processed, skipping");
                return result;
            }

            //RE::GPtr<RE::GFxMovieView> uiMovie

            lastUIEventRunTime = appRunTime;

            RE::InventoryMenu* subMenu = static_cast<RE::InventoryMenu*>(menu);

            ProcessUiItemMenuEvent(subMenu, message, uiItemMenuNames[UiItemMenuEnum_InventoryMenu]);
            return result;
        };

        static inline std::uint32_t idx = 0x4;

        static inline REL::Relocation<decltype(thunk)> func;

        // Install our hook at the specified address
        static inline void Install() {
            if (!inventoryMenuHookInstalled) {
                inventoryMenuHookInstalled = true;
                stl::write_vfunc<0, InventoryMenuHook>();
                logger::debug("installing inventory menu hook");
            }
        }
    };

    struct BarterMenuHook {
        inline static constexpr auto VTABLE = RE::VTABLE_BarterMenu;

        static RE::UI_MESSAGE_RESULTS thunk(RE::IMenu* menu, RE::UIMessage& message) {
            uint32_t appRunTime = RE::GetDurationOfApplicationRunTime();
            auto result = func(menu, message);

            //logger::trace("Barter event: appRunTime[{}] lastUIEventRunTime[{}]", appRunTime, lastUIEventRunTime);

            //ui events can triggered multiple times when opening or closing the menu. 
            //if it already ran then skip
            if (lastUIEventRunTime == appRunTime) {
                //logger::trace("Barter event: event already processed, skipping");
                return result;
            }

            lastUIEventRunTime = appRunTime;

            RE::BarterMenu* subMenu = static_cast<RE::BarterMenu*>(menu);
            ProcessUiItemMenuEvent(subMenu, message, uiItemMenuNames[UiItemMenuEnum_BarterMenu]);
            return result;
        };

        static inline std::uint32_t idx = 0x4;

        static inline REL::Relocation<decltype(thunk)> func;

        // Install our hook at the specified address
        static inline void Install() {
            if (!barterMenuHookInstalled) {
                barterMenuHookInstalled = true;
                stl::write_vfunc<0, BarterMenuHook>();
                logger::debug("installing barter menu hook");
            }
        }
    };

    struct ContainerMenuHook {
        inline static constexpr auto VTABLE = RE::VTABLE_ContainerMenu;

        static RE::UI_MESSAGE_RESULTS thunk(RE::IMenu* menu, RE::UIMessage& message) {
            uint32_t appRunTime = RE::GetDurationOfApplicationRunTime();
            auto result = func(menu, message);

            //logger::trace("Container event: appRunTime[{}] lastUIEventRunTime[{}]", appRunTime, lastUIEventRunTime);

            //ui events can triggered multiple times when opening or closing the menu. 
            //if it already ran then skip
            if (lastUIEventRunTime == appRunTime) {
                //logger::trace("Container event: event already processed, skipping");
                return result;
            }

            lastUIEventRunTime = appRunTime;

            RE::ContainerMenu* subMenu = static_cast<RE::ContainerMenu*>(menu);
            ProcessUiItemMenuEvent(subMenu, message, uiItemMenuNames[UiItemMenuEnum_ContainerMenu]);
            return result;
        };

        static inline std::uint32_t idx = 0x4;

        static inline REL::Relocation<decltype(thunk)> func;

        // Install our hook at the specified address
        static inline void Install() {
            if (!containerMenuHookInstalled) {
                containerMenuHookInstalled = true;
                stl::write_vfunc<0, ContainerMenuHook>();
                logger::debug("installing Container menu hook");
            }
        }
    };

    struct GiftMenuHook {
        inline static constexpr auto VTABLE = RE::VTABLE_GiftMenu;

        static RE::UI_MESSAGE_RESULTS thunk(RE::IMenu* menu, RE::UIMessage& message) {
            uint32_t appRunTime = RE::GetDurationOfApplicationRunTime();
            auto result = func(menu, message);

            //logger::trace("Gift event: appRunTime[{}] lastUIEventRunTime[{}]", appRunTime, lastUIEventRunTime);

            //ui events can triggered multiple times when opening or closing the menu. 
            //if it already ran then skip
            if (lastUIEventRunTime == appRunTime) {
                //logger::trace("Gift event: event already processed, skipping");
                return result;
            }

            lastUIEventRunTime = appRunTime;

            RE::GiftMenu* subMenu = static_cast<RE::GiftMenu*>(menu);
            ProcessUiItemMenuEvent(subMenu, message, uiItemMenuNames[UiItemMenuEnum_GiftMenu]);
            return result;
        };

        static inline std::uint32_t idx = 0x4;

        static inline REL::Relocation<decltype(thunk)> func;

        // Install our hook at the specified address
        static inline void Install() {
            if (!giftMenuHookInstalled) {
                giftMenuHookInstalled = true;
                stl::write_vfunc<0, GiftMenuHook>();
                logger::debug("installing Gift menu hook");
            }
        }
    };

    struct MagicMenuHook {
        inline static constexpr auto VTABLE = RE::VTABLE_MagicMenu;

        static RE::UI_MESSAGE_RESULTS thunk(RE::IMenu* menu, RE::UIMessage& message) {
            uint32_t appRunTime = RE::GetDurationOfApplicationRunTime();
            auto result = func(menu, message);
            
            //logger::trace("Magic event: appRunTime[{}] lastUIEventRunTime[{}]", appRunTime, lastUIEventRunTime);

            //ui events can triggered multiple times when opening or closing the menu. 
            //if it already ran then skip
            if (lastUIEventRunTime == appRunTime) {
                //logger::trace("Magic event: event already processed, skipping");
                return result;
            }

            lastUIEventRunTime = appRunTime;

            RE::MagicMenu* subMenu = static_cast<RE::MagicMenu*>(menu);
            ProcessUiMagicMenuEvent(subMenu, message, uiItemMenuNames[UiItemMenuEnum_MagicMenu]);
            return result;
        };

        static inline std::uint32_t idx = 0x4;

        static inline REL::Relocation<decltype(thunk)> func;

        // Install our hook at the specified address
        static inline void Install() {
            if (!magicMenuHookInstalled) {
                magicMenuHookInstalled = true;
                stl::write_vfunc<0, MagicMenuHook>();
                logger::debug("installing Magic menu hook");
            }
        }
    };

    struct CraftingMenuHook {
        inline static constexpr auto VTABLE = RE::VTABLE_CraftingMenu;

        static RE::UI_MESSAGE_RESULTS thunk(RE::IMenu* menu, RE::UIMessage& message) {
            uint32_t appRunTime = RE::GetDurationOfApplicationRunTime();
            auto result = func(menu, message);

            //logger::trace("Crafting event: appRunTime[{}] lastUIEventRunTime[{}]", appRunTime, lastUIEventRunTime);

            //ui events can triggered multiple times when opening or closing the menu. 
            //if it already ran then skip
            if (lastUIEventRunTime == appRunTime) {
                //logger::trace("Crafting event: event already processed, skipping");
                return result;
            }

            lastUIEventRunTime = appRunTime;

            RE::CraftingMenu* subMenu = static_cast<RE::CraftingMenu*>(menu);
            ProcessUiCraftingMenuEvent(subMenu, message, uiItemMenuNames[UiItemMenuEnum_Crafting_Menu]);
            return result;
        };

        static inline std::uint32_t idx = 0x4;

        static inline REL::Relocation<decltype(thunk)> func;

        // Install our hook at the specified address
        static inline void Install() {
            if (!craftingMenuHookInstalled) {
                craftingMenuHookInstalled = true;
                stl::write_vfunc<0, CraftingMenuHook>();
                logger::debug("installing Crafting menu hook");
            }
        }
    };

    struct FavoritesMenuHook {
        inline static constexpr auto VTABLE = RE::VTABLE_FavoritesMenu;

        static RE::UI_MESSAGE_RESULTS thunk(RE::IMenu* menu, RE::UIMessage& message) {
            uint32_t appRunTime = RE::GetDurationOfApplicationRunTime();
            auto result = func(menu, message);
            //logger::trace("Favorites event: appRunTime[{}] lastUIEventRunTime[{}]", appRunTime, lastUIEventRunTime);

            //this Favorites event is triggered multiple times when opening or closing the menu. 
            //if it already ran then skip
            if (lastUIEventRunTime == appRunTime) {
                //logger::trace("Favorites event: event already processed, skipping");
                return result;
            }

            lastUIEventRunTime = appRunTime;

            RE::FavoritesMenu* subMenu = static_cast<RE::FavoritesMenu*>(menu);

            ProcessUiFavoritesMenuEvent(subMenu, message, uiItemMenuNames[UiItemMenuEnum_FavoritesMenu]);
            return result;
        };

        static inline std::uint32_t idx = 0x4;

        static inline REL::Relocation<decltype(thunk)> func;

        // Install our hook at the specified address
        static inline void Install() {
            if (!favoritesMenuHookInstalled) {
                favoritesMenuHookInstalled = true;
                stl::write_vfunc<0, FavoritesMenuHook>();
                logger::debug("installing Favorites menu hook");
            }
        }
    };

    void InstallUiEventHook(int index) {
        logger::trace("called. index = {}", index);
        switch (index) {
            
        case UiItemMenuEnum_InventoryMenu:
            InventoryMenuHook::Install();
            break;

        case UiItemMenuEnum_BarterMenu:
            BarterMenuHook::Install();
            break;

        case UiItemMenuEnum_ContainerMenu:
            ContainerMenuHook::Install();
            break;

        case UiItemMenuEnum_GiftMenu:
            GiftMenuHook::Install();
            break;

        case UiItemMenuEnum_MagicMenu:
            MagicMenuHook::Install();
            break;

        case UiItemMenuEnum_Crafting_Menu:
            CraftingMenuHook::Install();
            break;

        case UiItemMenuEnum_FavoritesMenu:
            FavoritesMenuHook::Install();
            break;

        default:
            logger::trace("installing all hooks");
            InventoryMenuHook::Install();
            BarterMenuHook::Install();
            ContainerMenuHook::Install();
            GiftMenuHook::Install();
            MagicMenuHook::Install();
            CraftingMenuHook::Install();
            FavoritesMenuHook::Install();
            break;
        }
    }

    int GetIndexInVector(std::vector<UIEventData>& v, UIEventData& element) {
        if (v.size() == 0) {
            return -1;
        }

        for (int i = 0; i < v.size(); i++) {
            if (v[i] == element) {
                return i;
            }
        }

        return -1;
    }
    
    void UnregisterHandleForUiItemEvent_All(RE::VMHandle handle) {
        isUnregistering_All = true;
        int size = registeredUIEventDatas.size();
        if (size == 0) {
            return;
        }

        int i = 0;

        while (i < size) {
            if (registeredUIEventDatas[i].registeredHandle == handle) {
                registeredUIEventDatas.erase(registeredUIEventDatas.begin() + i);
                size--;
            }
            else {
                i++;
            }
        }
        isUnregistering_All = false;
    }

    //form ==========================================================================================================================================================================================
    bool IsFormRegisteredForUiItemMenuEvent(RE::StaticFunctionTag*, std::string menuName, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int eventType) {
        if (!gfuncs::IsFormValid(eventReceiver)) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return false;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", gfuncs::GetFormName(eventReceiver));
            return false;
        }

        if (menuName != "") {
            std::transform(menuName.begin(), menuName.end(), menuName.begin(), tolower);
            int menuNameIndex = gfuncs::GetIndexInVector(uiItemMenuNames, menuName);
            if (menuNameIndex == -1) {
                logger::error("menuName [{}] not recognized", menuName);
                return false;
            }
        }

        UIEventData data(handle, menuName, paramFilter, eventType);
        return (GetIndexInVector(registeredUIEventDatas, data) != -1);
    }

    void RegisterFormForUiItemMenuEvent(RE::StaticFunctionTag*, std::string menuName, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int eventType) {
        if (!gfuncs::IsFormValid(eventReceiver)) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", gfuncs::GetFormName(eventReceiver));
            return;
        }

        int menuNameIndex = -1;

        if (menuName != "") {
            std::transform(menuName.begin(), menuName.end(), menuName.begin(), tolower);
            menuNameIndex = gfuncs::GetIndexInVector(uiItemMenuNames, menuName);
            if (menuNameIndex == -1) {
                logger::error("menuName [{}] not recognized", menuName);
                return;
            }
        }

        UIEventData data(handle, menuName, paramFilter, eventType);
        int index = GetIndexInVector(registeredUIEventDatas, data);
        if (index == -1) {
            registeredUIEventDatas.push_back(data);
            InstallUiEventHook(menuNameIndex);
            logger::trace("registered menu[{}] eventReceiver[{}] paramFilter[{}] eventType[{}]",
                __func__, menuName, gfuncs::GetFormName(eventReceiver), gfuncs::GetFormName(paramFilter), eventType);
        }
    }

    void UnregisterFormForUiItemMenuEvent(RE::StaticFunctionTag*, std::string menuName, RE::TESForm* eventReceiver, RE::TESForm* paramFilter, int eventType) {
        if (!gfuncs::IsFormValid(eventReceiver)) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", gfuncs::GetFormName(eventReceiver));
            return;
        }

        if (menuName != "") {
            std::transform(menuName.begin(), menuName.end(), menuName.begin(), tolower);
            int menuNameIndex = gfuncs::GetIndexInVector(uiItemMenuNames, menuName);
            if (menuNameIndex == -1) {
                logger::error("menuName [{}] not recognized", menuName);
                return;
            }
        }

        UIEventData data(handle, menuName, paramFilter, eventType);
        int index = GetIndexInVector(registeredUIEventDatas, data);
        if (index > -1) {
            registeredUIEventDatas.erase(registeredUIEventDatas.begin() + index);
            logger::trace("Unregistered menu[{}] eventReceiver[{}] paramFilter[{}] eventType[{}]",
                __func__, menuName, gfuncs::GetFormName(eventReceiver), gfuncs::GetFormName(paramFilter), eventType);
        }
    }

    void UnregisterFormForUiItemMenuEvent_All(RE::StaticFunctionTag*, RE::TESForm* eventReceiver) {
        
        if (!gfuncs::IsFormValid(eventReceiver)) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", gfuncs::GetFormName(eventReceiver));
            return;
        }

        std::thread t([=]() {
            while (isUnregistering_All) {
                //this function should only be running once at a time
                std::this_thread::sleep_for(std::chrono::milliseconds(gfuncs::GetRandomInt(50, 100)));
            }
            UnregisterHandleForUiItemEvent_All(handle);
            });
        t.detach();
    }

    //Alias ==========================================================================================================================================================================================
    bool IsAliasRegisteredForUiItemMenuEvent(RE::StaticFunctionTag*, std::string menuName, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int eventType) {
        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return false;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", eventReceiver->aliasName);
            return false;
        }

        if (menuName != "") {
            std::transform(menuName.begin(), menuName.end(), menuName.begin(), tolower);
            int menuNameIndex = gfuncs::GetIndexInVector(uiItemMenuNames, menuName);
            if (menuNameIndex == -1) {
                logger::error("menuName [{}] not recognized", menuName);
                return false;
            }
        }

        UIEventData data(handle, menuName, paramFilter, eventType);
        return (GetIndexInVector(registeredUIEventDatas, data) != -1);
    }

    void RegisterAliasForUiItemMenuEvent(RE::StaticFunctionTag*, std::string menuName, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int eventType) {
        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", eventReceiver->aliasName);
            return;
        }

        int menuNameIndex = -1;

        if (menuName != "") {
            std::transform(menuName.begin(), menuName.end(), menuName.begin(), tolower);
            menuNameIndex = gfuncs::GetIndexInVector(uiItemMenuNames, menuName);
            if (menuNameIndex == -1) {
                logger::error("menuName [{}] not recognized", menuName);
                return;
            }
        }

        UIEventData data(handle, menuName, paramFilter, eventType);
        int index = GetIndexInVector(registeredUIEventDatas, data);
        if (index == -1) {
            registeredUIEventDatas.push_back(data);
            InstallUiEventHook(menuNameIndex);
            logger::trace("registered menu[{}] eventReceiver[{}] paramFilter[{}] eventType[{}]",
                __func__, menuName, eventReceiver->aliasName, gfuncs::GetFormName(paramFilter), eventType);
        }
    }

    void UnregisterAliasForUiItemMenuEvent(RE::StaticFunctionTag*, std::string menuName, RE::BGSBaseAlias* eventReceiver, RE::TESForm* paramFilter, int eventType) {
        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", eventReceiver->aliasName);
            return;
        }

        if (menuName != "") {
            std::transform(menuName.begin(), menuName.end(), menuName.begin(), tolower);
            int menuNameIndex = gfuncs::GetIndexInVector(uiItemMenuNames, menuName);
            if (menuNameIndex == -1) {
                logger::error("menuName [{}] not recognized", menuName);
                return;
            }
        }

        UIEventData data(handle, menuName, paramFilter, eventType);
        int index = GetIndexInVector(registeredUIEventDatas, data);
        if (index > -1) {
            registeredUIEventDatas.erase(registeredUIEventDatas.begin() + index);
            logger::trace("Unregistered menu[{}] eventReceiver[{}] paramFilter[{}] eventType[{}]",
                __func__, menuName, eventReceiver->aliasName, gfuncs::GetFormName(paramFilter), eventType);
        }
    }

    void UnregisterAliasForUiItemMenuEvent_All(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver) {

        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", eventReceiver->aliasName);
            return;
        }

        std::thread t([=]() {
            while (isUnregistering_All) {
                //this function should only be running once at a time
                std::this_thread::sleep_for(std::chrono::milliseconds(gfuncs::GetRandomInt(50, 100)));
            }
            UnregisterHandleForUiItemEvent_All(handle);
            });
        t.detach();
    }

    //Alias ==========================================================================================================================================================================================
    bool IsActiveMagicEffectRegisteredForUiItemMenuEvent(RE::StaticFunctionTag*, std::string menuName, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int eventType) {
        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return false;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()));
            return false;
        }

        if (menuName != "") {
            std::transform(menuName.begin(), menuName.end(), menuName.begin(), tolower);
            int menuNameIndex = gfuncs::GetIndexInVector(uiItemMenuNames, menuName);
            if (menuNameIndex == -1) {
                logger::error("menuName [{}] not recognized", menuName);
                return false;
            }
        }

        UIEventData data(handle, menuName, paramFilter, eventType);
        return (GetIndexInVector(registeredUIEventDatas, data) != -1);
    }

    void RegisterActiveMagicEffectForUiItemMenuEvent(RE::StaticFunctionTag*, std::string menuName, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int eventType) {
        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()));
            return;
        }

        int menuNameIndex = -1;

        if (menuName != "") {
            std::transform(menuName.begin(), menuName.end(), menuName.begin(), tolower);
            menuNameIndex = gfuncs::GetIndexInVector(uiItemMenuNames, menuName);
            if (menuNameIndex == -1) {
                logger::error("menuName [{}] not recognized", menuName);
                return;
            }
        }

        UIEventData data(handle, menuName, paramFilter, eventType);
        int index = GetIndexInVector(registeredUIEventDatas, data);
        if (index == -1) {
            registeredUIEventDatas.push_back(data);
            InstallUiEventHook(menuNameIndex);
            logger::trace("registered menu[{}] eventReceiver[{}] paramFilter[{}] eventType[{}]",
                __func__, menuName, gfuncs::GetFormName(eventReceiver->GetBaseObject()), gfuncs::GetFormName(paramFilter), eventType);
        }
    }

    void UnregisterActiveMagicEffectForUiItemMenuEvent(RE::StaticFunctionTag*, std::string menuName, RE::ActiveEffect* eventReceiver, RE::TESForm* paramFilter, int eventType) {
        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()));
            return;
        }

        if (menuName != "") {
            std::transform(menuName.begin(), menuName.end(), menuName.begin(), tolower);
            int menuNameIndex = gfuncs::GetIndexInVector(uiItemMenuNames, menuName);
            if (menuNameIndex == -1) {
                logger::error("menuName [{}] not recognized", menuName);
                return;
            }
        }

        UIEventData data(handle, menuName, paramFilter, eventType);
        int index = GetIndexInVector(registeredUIEventDatas, data);
        if (index > -1) {
            registeredUIEventDatas.erase(registeredUIEventDatas.begin() + index);
            logger::trace("Unregistered menu[{}] eventReceiver[{}] paramFilter[{}] eventType[{}]",
                __func__, menuName, gfuncs::GetFormName(eventReceiver->GetBaseObject()), gfuncs::GetFormName(paramFilter), eventType);
        }
    }

    void UnregisterActiveMagicEffectForUiItemMenuEvent_All(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver) {

        if (!eventReceiver) {
            logger::warn("eventReceiver doesn't exist or isn't valid");
            return;
        }

        RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

        if (handle == NULL) {
            logger::error("couldn't get handle for eventReceiver[{}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()));
            return;
        }

        std::thread t([=]() {
            while (isUnregistering_All) {
                //this function should only be running once at a time
                std::this_thread::sleep_for(std::chrono::milliseconds(gfuncs::GetRandomInt(50, 100)));
            }
            UnregisterHandleForUiItemEvent_All(handle);
            });
        t.detach();
    }

    void Install() {
        if (!playerRef) { playerRef = RE::TESForm::LookupByID<RE::TESForm>(20)->As<RE::Actor>(); }
        if (!ui) { ui = RE::UI::GetSingleton(); }
        if (!userEvents) { userEvents = RE::UserEvents::GetSingleton(); }

        if (userEvents && validSelectUserEventStrings.size() == 0) {
            validSelectUserEventStrings.push_back(userEvents->rightEquip);
            validSelectUserEventStrings.push_back(userEvents->leftEquip); 
            validSelectUserEventStrings.push_back(userEvents->unk318);
            validSelectUserEventStrings.push_back(userEvents->accept);
            validSelectUserEventStrings.push_back(userEvents->equip);
            validSelectUserEventStrings.push_back(userEvents->unequip);
            validSelectUserEventStrings.push_back(userEvents->rightAttack);
            validSelectUserEventStrings.push_back(userEvents->leftAttack);
            validSelectUserEventStrings.push_back(userEvents->click);
            validSelectUserEventStrings.push_back(userEvents->activate);

            validDropUserEventStrings.push_back(userEvents->xButton);
            validDropUserEventStrings.push_back(userEvents->dropItem);

            validFavoriteUserEventStrings.push_back(userEvents->yButton);
            validFavoriteUserEventStrings.push_back(userEvents->toggleFavorite);
        }

        logger::trace("UI menu hooks initialized");
    }
}
