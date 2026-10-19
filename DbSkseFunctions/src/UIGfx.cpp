#include "UIGfx.h"
#include "GeneralFunctions.h"
#include "RE/B/BookMenu.h"
#include "SharedVariables.h"
#include <winbase.h>

namespace gfx {
    std::string GetGFxTypeString(int type) {
        switch (type) {
            case 0: { return "Undefined"; }
            case 1: { return "Null"; }
            case 2: { return "Bool"; }
            case 3: { return "Number"; }
            case 4: { return "String"; }
            case 5: { return "StringW"; }
            case 6: { return "Object"; }
            case 7: { return "Array"; }
            case 8: { return "DisplayObject"; }
        }
        return ("unrecognize type [" + std::to_string(type) + "]");
    }

    std::string GetGfxValueAsString(RE::GFxValue& gfxValue) {
        std::string returnString;
        if (gfxValue.IsString()) {
            returnString = gfxValue.GetString();
        }
        else if (gfxValue.IsStringW()) {
            const wchar_t* txt = gfxValue.GetStringW();
            std::wstring ws(txt);
            std::string str(ws.begin(), ws.end());
            returnString = str;
        }
        else if (gfxValue.IsNumber()) {
            returnString = std::to_string(gfxValue.GetNumber());
        }
        else if (gfxValue.IsBool()) {
            returnString = std::to_string(gfxValue.GetBool());
        }
        /*else if (gfxValue.IsObject()) {
            returnString = "object";
        }
        else if (gfxValue.IsDisplayObject()) {
            returnString = "display object";
        }
        else if (gfxValue.IsArray()) {
            returnString = "array";
        }*/
        return returnString;
    }

    std::string GetGfxValueAsString(const RE::GFxValue& gfxValue) {
        std::string returnString = "";
        if (gfxValue.IsString()) {
            returnString = gfxValue.GetString();
        }
        else if (gfxValue.IsStringW()) {
            const wchar_t* txt = gfxValue.GetStringW();
            std::wstring ws(txt);
            std::string str(ws.begin(), ws.end());
            returnString = str;
        }
        else if (gfxValue.IsNumber()) {
            returnString = std::to_string(gfxValue.GetNumber());
        }
        else if (gfxValue.IsBool()) {
            returnString = std::to_string(gfxValue.GetBool());
        }
        /*else if (gfxValue.IsObject()) {
            returnString = "object";
        }
        else if (gfxValue.IsDisplayObject()) {
            returnString = "display object";
        }
        else if (gfxValue.IsArray()) {
            returnString = "array";
        }*/
        return returnString;
    }

    std::string GetItemListPathForItemMenu(std::string_view menuName) {
        if (menuName == RE::CraftingMenu::MENU_NAME) {
            return "_root.Menu.InventoryLists.panelContainer.itemList";
        }
        else if (menuName == RE::FavoritesMenu::MENU_NAME) {
            return "_root.MenuHolder.Menu_mc.itemList";
        }
        else {
            //for these menus :
           //"inventorymenu"
           //"bartermenu"
           //"containermenu"
           //"giftmenu"
           //"magicmenu"
            return "_root.Menu_mc.inventoryLists.itemList";
        }
    }

    void InvokeInt(std::string_view menuPath, std::string target, int arg) {
		auto* bssVm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		
        if (bssVm) {
            auto* args = RE::MakeFunctionArguments((std::string)menuPath, (std::string)target, (int)arg);
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> result;
            result.reset();
            bssVm->DispatchStaticCall("UI", "InvokeInt", args, result);
            delete args;
        }
    }

    void SetItemMenuSelection(std::string_view menuPath, int index) {
        //std::string path = (gfx::GetItemListPathForItemMenu(menuPath) + ".ItemsListEntry" + std::to_string(topDownIndex) + ".this._parent.onItemRollOver");
        std::string path = (gfx::GetItemListPathForItemMenu(menuPath) + ".onItemRollOver");
        
        InvokeInt(menuPath, path, index);
    }

    bool IsGfxMemberValid(const RE::GFxValue& gfxValue, std::string gfxName) {
        if (gfxValue == NULL) {
            //logger::error("gfxValue [{}] is NULL", gfxName);
            return false;
        }
        else if (gfxValue.IsNull()) {
            //logger::error("gfxValue [{}] is IsNull", gfxName);
            return false;
        }
        else if (gfxValue.IsUndefined()) {
            //logger::error("gfxValue [{}] is IsUndefined", gfxName);
            return false;
        }
        return true;
    }

    bool IsGfxMemberValid(RE::GFxValue& gfxValue, std::string gfxName) {
        if (gfxValue == NULL) {
            //logger::error("gfxValue [{}] is NULL", gfxName);
            return false;
        }
        else if (gfxValue.IsNull()) {
            //logger::error("gfxValue [{}] is IsNull", gfxName);
            return false;
        }
        else if (gfxValue.IsUndefined()) {
            //logger::error("gfxValue [{}] is IsUndefined", gfxName);
            return false;
        }
        return true;
    }

    bool CanGfxValueVisitMembers(RE::GFxValue& gfxValue) {
        if (gfxValue == NULL) {
            return false;
        }
        int gfxtype = static_cast<int>(gfxValue.GetType());
        //type is bool, number, string or stringW
        if (gfxtype >= 2 && gfxtype <= 5) {
            return false;
        }
        return true;
    }

    bool CanGfxValueVisitMembers(const RE::GFxValue& gfxValue) {
        if (gfxValue == NULL) {
            return false;
        }
        int gfxtype = static_cast<int>(gfxValue.GetType());
        //type is bool, number, string or stringW
        if (gfxtype >= 2 && gfxtype <= 5) {
            return false;
        }
        return true;
    }

    bool GFxMemberNameIsValid(std::string name) {
        if (name.find("constraints") != std::string::npos) {
            return false;
        }
        if (name.find("scope") != std::string::npos) {
            return false;
        }
        if (name.find("track") != std::string::npos) {
            return false;
        }
        if (name.find("focusTarget") != std::string::npos) {
            return false;
        }

        if (gfuncs::StringHasNCharacters(name, '.', 10)) {
            return false;
        }

        std::string::difference_type n = std::count(name.begin(), name.end(), '.');

        // optional: convert to int
        int count = static_cast<int>(n);

        return true;
    }

    std::vector<std::pair<const char*, RE::GFxValue>> GetGFxMembers(const RE::GFxValue& gfxValue) {
        std::vector<std::pair<const char*, RE::GFxValue>> members;

        int gfxtype = static_cast<int>(gfxValue.GetType());
        if (CanGfxValueVisitMembers(gfxValue)) {
            gfxValue.VisitMembers([&]([[maybe_unused]] const char* name, const RE::GFxValue& value) {
                std::pair<const char*, RE::GFxValue> member(name, value);
                members.push_back(member);
            });
        }
        return members;
    }

    
    void LogGFxMembers(const RE::GFxValue& gfxValue, std::string gfxName) {
        if (gfxValue == NULL) {
            logger::error("[{}] not found", gfxName);
            return;
        }

        std::string gfxValueString = GetGfxValueAsString(gfxValue);
        int gfxtype = static_cast<int>(gfxValue.GetType());
        logger::info("logging gfxValue: type[{}] valueString[{}] gfxValue[{}] ", GetGFxTypeString(gfxtype), gfxValueString, gfxName);

        //type not bool, number, string or stringW
        if (CanGfxValueVisitMembers(gfxValue)) {
            gfxValue.VisitMembers([&]([[maybe_unused]] const char* name, const RE::GFxValue& value) {
                //gfxValue.VisitMembers([&](const char* name, const RE::GFxValue& value) {
                int type = -1;
                std::string valueString = "null";
                std::string memberName = (gfxName + "." + name);

                if (value != NULL) {
                    type = static_cast<int>(value.GetType());
                    valueString = GetGfxValueAsString(value);
                }
                logger::info("type[{}] value[{}] member[{}] ", GetGFxTypeString(type), valueString, memberName);
            });
        } 
    }

    void LogGFxMembers(RE::GPtr<RE::GFxMovieView> mv, std::vector<std::string> memberStrings) {
        if (mv) {
            auto size = memberStrings.size();
            for (int i = 0; i < size; i++) {
                RE::GFxValue gfxValue;
                std::string name = memberStrings[i];
                if (mv->GetVariable(&gfxValue, name.c_str())) {
                    LogGFxMembers(gfxValue, name);
                }
                else {
                    logger::error("failed to get gfxValue for [{}]", name);
                }
                logger::info("\n\n");
            }
        }
    }

    void EraseQuantityStringFromUIitemName(std::string& uiItemName) {
        if (uiItemName != "") {
            int iStart = uiItemName.find(" (");
            if (iStart != std::string::npos) {
                int iEnd = uiItemName.find(")", iStart);
                if (iEnd != std::string::npos) {
                    int subStringStart = (iStart + 2);
                    std::string subString = uiItemName.substr(subStringStart, (iEnd - subStringStart));
                    if (gfuncs::IsDecString(subString)) {
                        uiItemName.erase(iStart, (iEnd - iStart + 1));
                    }
                }
            }
        }
    }

    //if the GFxValue has a member named "text" get its string value
    std::string GetGFxListEntryText(RE::GFxValue& listEntry) {
        std::string text = "";
        if (listEntry == NULL) {
            return text;
        }

        int gfxtype = static_cast<int>(listEntry.GetType());
        std::string sText = "text";
        //const char* cText = sText.c_str();
        //type not bool, number, string or stringW
        if (CanGfxValueVisitMembers(listEntry)) {
            listEntry.VisitMembers([&]([[maybe_unused]] const char* name, const RE::GFxValue& value) {
                //gfxValue.VisitMembers([&](const char* name, const RE::GFxValue& value) {

                std::string sName(name);
                if (sName == sText) {
                    if (value != NULL) {
                        if (value.IsString()) {
                            text = value.GetString();
                            logger::info("name[{}] text[{}]", name, text);
                        }
                    }
                }
            });
        }
        return text;
    } 

    int GetEntryDataArrayLength(std::string_view menuName, RE::GPtr<RE::GFxMovieView>mv) {
        std::string path = GetItemListPathForItemMenu(menuName) + ".listEnumeration._entryData.length";
        if (mv) {
            RE::GFxValue gfxArrayLength;
            if (mv->GetVariable(&gfxArrayLength, path.c_str())) {
                if (gfxArrayLength.IsNumber()) {
                    return  int(gfxArrayLength.GetNumber());
                }
            }
        }
        return 0;
    } 

    int GetEntryDataArrayLength(std::string_view menuName) {
		auto* ui = RE::UI::GetSingleton();
        if (ui) {
            auto mv = ui->GetMovieView(menuName);
            return GetEntryDataArrayLength(menuName, mv);
        }
        return 0;
    }

    int GetIndexForMenuItem(std::string_view menuName, std::string sItemName) {
        std::string path = GetItemListPathForItemMenu(menuName);
        //logger::trace("called");

        //sUIitemListEntryPath will be something like "_root.Menu_mc.inventoryLists.itemList.ItemsListEntry"
        //std::string sUIitemListEntryPath = path + ".listEnumeration._entryData.";
        std::string sUIitemListEntryPath = path + ".listEnumeration._entryData.";

		auto* ui = RE::UI::GetSingleton();
        if (ui) {
            auto mv = ui->GetMovieView(menuName);
            if (mv) {
                
                int i = 0;
                int iLimit = GetEntryDataArrayLength(menuName, mv);
                //std::string lString = std::format(".listEnumeration._entryData.length = {}", iLimit);
                //logger::trace("{}", lString);
                //RE::DebugNotification(lString.c_str());
                
                while (i < iLimit) {
                    std::string entryPath = (sUIitemListEntryPath + (std::to_string(i)));
                    //RE::GFxValue entrygfx;
                    std::string textPath = entryPath + ".text";
                    RE::GFxValue textgfx;
                    std::string indexPath = entryPath + ".itemIndex";
                    RE::GFxValue indexgfx;
                    std::string uiItemName = "not found";
                    int itemIndexValue = -1;

                    //if (mv->GetVariable(&entrygfx, entryPath.c_str())) {
                    //    //LogGFxMembers(entrygfx, entryPath);
                    //    uiItemName = GetGFxListEntryText(entrygfx);
                    //}

                    if (mv->GetVariable(&indexgfx, indexPath.c_str())) {
                        if (indexgfx.IsNumber()) {
                            itemIndexValue = indexgfx.GetNumber();
                        }
                    }

                    if (mv->GetVariable(&textgfx, textPath.c_str())) {
                        if (textgfx != NULL) {
                            if (textgfx.IsString()) {
                                uiItemName = textgfx.GetString();
                                if (uiItemName == sItemName) {
                                    if (itemIndexValue > -1) {
                                        //logger::trace("uiItemName[{}], itemIndex[{}] i[{}]", uiItemName, itemIndexValue, i);
                                        return itemIndexValue;
                                    }
                                }
                            }
                        }
                    }

                    //logger::trace("uiItemName[{}], itemIndex[{}] i[{}]", uiItemName, itemIndexValue, i);
                    i++;
                }
            }
        }
        return -1;
    }

    int GetSelectedEntryIndex(std::string_view menuName) {
        int iReturn = -1;
		auto* ui = RE::UI::GetSingleton();
        if (ui) {
            auto mv = ui->GetMovieView(menuName);
            if (mv) {
                std::string selectedItemIndexPath = GetItemListPathForItemMenu(menuName) + ".selectedEntry.itemIndex";
                //std::string selectedItemIndexPath = GetItemListPathForItemMenu(menuName) + ".selectedIndex";
                RE::GFxValue gfxValue;
                if (mv->GetVariable(&gfxValue, selectedItemIndexPath.c_str())) {
                    if (gfxValue.IsNumber()) {
                        iReturn = gfxValue.GetNumber();
                    }
                }
            }
        } 
        return iReturn;
    }

    std::string GetSelectedEntryText(std::string_view menuName) {
        std::string sReturn = "";
		auto* ui = RE::UI::GetSingleton();
        if (ui) {
            auto mv = ui->GetMovieView(menuName);
            if (mv) {
                std::string selectedItemIndexPath = GetItemListPathForItemMenu(menuName) + ".selectedEntry.text";
                RE::GFxValue gfxValue;
                if (mv->GetVariable(&gfxValue, selectedItemIndexPath.c_str())) {
                    if (gfxValue.IsString()) {
                        sReturn = gfxValue.GetString();
                    }
                }
            }
        }
        return sReturn;
    }

    RE::GFxValue GetSelectedEntry(std::string_view menuName) {
        RE::GFxValue selectedEntryGfx;

		auto* ui = RE::UI::GetSingleton();
        if (ui) {
            auto mv = ui->GetMovieView(menuName);
            if (mv) {
                std::string selectedItemIndexPath = GetItemListPathForItemMenu(menuName) + ".selectedEntry";
                if (mv->GetVariable(&selectedEntryGfx, selectedItemIndexPath.c_str())) {
                    //logger::trace("selected entry found");
                }
            }
        }
        return selectedEntryGfx;
    }

	RE::GPtr<RE::GFxMovieView> GetMenuMovie(std::string menuName) {
		auto* ui = RE::UI::GetSingleton();
		if (!ui) {
			return nullptr;
		}

		//"book" addresses the Book Menu's book.swf.
		if (_stricmp(menuName.c_str(), "book") == 0) {
			auto bookMenu = ui->GetMenu<RE::BookMenu>(RE::BookMenu::MENU_NAME);
			return bookMenu ? bookMenu->GetRuntimeData().book : nullptr;
		}

		auto menu = ui->GetMenu(menuName);
		if (menu) {
			return menu->uiMovie;
		}
		
		return nullptr;
	}

	std::pair<RE::GFxValue, bool> GetUITargetGfx(std::string menuName, std::string target) {
		RE::GFxValue gfx;
		RE::GPtr<RE::GFxMovieView> mv = GetMenuMovie(menuName);
		const bool got = mv ? mv->GetVariable(&gfx, target.c_str()) : false;
		return { gfx, got };
	}
	
    //papyrus functions====================================================================================================================
    int GetUiTargetType(RE::StaticFunctionTag*, std::string menuName, std::string target) {
        int gfxtype = -1;
        auto gfxp = GetUITargetGfx(menuName, target);
        if (gfxp.second == true) {
            gfxtype = static_cast<int>(gfxp.first.GetType());
        }
        return gfxtype;
    }

    std::string GetUiTargetTypeAsString(RE::StaticFunctionTag*, std::string menuName, std::string target) {
        auto gfxp = GetUITargetGfx(menuName, target);
        if (gfxp.second == true) {
            int gfxtype = static_cast<int>(gfxp.first.GetType());
            return GetGFxTypeString(gfxtype);
        }
        return "target not found";
    }

    std::string GetUiTargetValueAsString(RE::StaticFunctionTag*, std::string menuName, std::string target) {
        auto gfxp = GetUITargetGfx(menuName, target);
        if (gfxp.second == true) {
            return GetGfxValueAsString(gfxp.first);
        }
        return "";
    }

    std::vector<std::string> GetUiTargetMembers(RE::StaticFunctionTag*, std::string menuName, std::string target) {
        std::vector<std::string> members;
        auto gfxp = GetUITargetGfx(menuName, target);
        if (gfxp.second == true) {
            int gfxtype = static_cast<int>(gfxp.first.GetType());
            std::string sType = GetGFxTypeString(gfxtype);
            std::string sValue = GetGfxValueAsString(gfxp.first);
            logger::info("Getting members for menuName[{}] target[{}] type[{}] value[{}]", menuName, target, sType, sValue);

            if (gfxtype < 2 || gfxtype > 5) {
                gfxp.first.VisitMembers([&]([[maybe_unused]] const char* name, const RE::GFxValue& value) {
                    std::string s = { name };
                    members.push_back(s);
                    logger::info("member[{}]", s);
                });
            }
        }
        else {
            logger::warn("couldn't find menuName[{}] target[{}]", menuName, target);
        }
        return members;
    }

    std::vector<std::string> GetUiTargetMembersData(RE::StaticFunctionTag*, std::string menuName, std::string target) {
        std::vector<std::string> members;
        auto gfxp = GetUITargetGfx(menuName, target);
        if (gfxp.second == true) {
            int gfxtype = static_cast<int>(gfxp.first.GetType());
            std::string sType = GetGFxTypeString(gfxtype);
            std::string sValue = GetGfxValueAsString(gfxp.first);
            logger::info("Getting data for menuName[{}] target[{}] type[{}] value[{}]", menuName, target, sType, sValue);

            if (gfxtype < 2 || gfxtype > 5) {
                gfxp.first.VisitMembers([&]([[maybe_unused]] const char* name, const RE::GFxValue& value) {
                    gfxtype = static_cast<int>(gfxp.first.GetType());
                    sType = GetGFxTypeString(gfxtype);
                    sValue = GetGfxValueAsString(value);
                    std::string sName = { name };
                    sName = (target + "." + sName);
                    std::string s = std::format("type[{}] value[{}] member[{}]", sType, sValue, sName);
                    members.push_back(s);
                    logger::info("{}", s);
                });
            }
        }
        else {
            logger::warn("couldn't find menuName[{}] target[{}]", menuName, target);
        }
        return members;
    }

	//Members that point back up or sideways in the tree. Following them turns the walk into
	//a loop (_parent -> child -> _parent ...) or wanders the whole movie via _root/_global.
	const std::unordered_set<std::string_view> kSkipMembers = {
		"_parent", "_root", "_global", "_level0", "this",
		"__proto__", "prototype", "constructor", "__constructor__", "__resolve"
	};

	bool IsTraversable(const RE::GFxValue& a_value) {
		return a_value.IsObject() || a_value.IsArray() || a_value.IsDisplayObject();
	}

	void FindMemberRecursive(const RE::GFxValue& a_node, const std::string& a_path, const char* a_memberName,
	int a_depth, int a_maxDepth, std::size_t& a_visited, std::size_t a_maxVisited, std::vector<std::string>& a_results) {
		
		if (a_depth > a_maxDepth || a_visited >= a_maxVisited) {
			return;
		}
		++a_visited;

		//HasMember follows the prototype chain, like `in` in ActionScript. That matters here:
		//class methods such as SetBookText live on the class prototype, not on the instance,
		//so VisitMembers on the instance may never list them -- but HasMember still finds them.
		if (a_node.HasMember(a_memberName)) {
			RE::GFxValue found;
			std::string sType = "unknown";
			if (a_node.GetMember(a_memberName, &found)) {
				sType = GetGFxTypeString(static_cast<int>(found.GetType()));
			}
			std::string s = std::format("{}.{} type[{}]", a_path, a_memberName, sType);
			logger::trace("{}", s);
			a_results.push_back(std::move(s));
		}

		a_node.VisitMembers([&](const char* a_name, const RE::GFxValue& a_value) {
			if (!a_name || kSkipMembers.contains(a_name) || !IsTraversable(a_value)) {
				return;
			}
			FindMemberRecursive(a_value, a_path + "." + a_name, a_memberName,
				a_depth + 1, a_maxDepth, a_visited, a_maxVisited, a_results);
		});
	}

	//Search any GFx movie, starting at startPath (e.g. "_root"), for objects that have memberName.
	std::vector<std::string> FindMovieMembers(RE::GFxMovieView* a_movie, const std::string& a_startPath, const std::string& a_memberName, int a_maxDepth) {
		std::vector<std::string> results;

		if (!a_movie) {
			// logger::warn("no movie");
			return results;
		}

		RE::GFxValue start;
		if (!a_movie->GetVariable(&start, a_startPath.c_str()) || !IsTraversable(start)) {
			// logger::warn("[{}] not found or not an object", a_startPath);
			return results;
		}

		constexpr std::size_t kMaxVisited = 5000;
		std::size_t visited = 0;

		FindMemberRecursive(start, a_startPath, a_memberName.c_str(),
			0, (a_maxDepth > 0 ? a_maxDepth : 6), visited, kMaxVisited, results);

		std::sort(results.begin(), results.end());
		results.erase(std::unique(results.begin(), results.end()), results.end());

		logger::trace("FindMovieMember start[{}] member[{}]: visited[{}] objects, found[{}]",
			a_startPath, a_memberName, visited, results.size());

		return results;
	}
	
	std::vector<std::string> FindUiMember(RE::StaticFunctionTag*, std::string menuName, std::string target, std::string memberName, int maxDepth) {
		if (menuName.empty() || target.empty() || memberName.empty()) {
			return {};
		}

		auto mv = GetMenuMovie(menuName);
		if (!mv) {
			logger::warn("menu [{}] is not open", menuName);
			return {};
		}

		return FindMovieMembers(mv.get(), target, memberName, maxDepth);
	}

		//Images used by SetBookMenuText that the book's original text didn't have.
	//All access is on the UI thread (UI tasks and MenuOpenCloseEvent), so no lock is needed.
	namespace bookImages {
		std::vector<std::unique_ptr<RE::BSScaleformExternalTexture>> extraTextures;

		RE::BSScaleformImageLoader* GetImageLoader() {
			auto* sfm = RE::BSScaleformManager::GetSingleton();
			return sfm ? sfm->imageLoader.get() : nullptr;
		}

		void ReleaseAll() {
			auto* loader = GetImageLoader();
			for (auto& tex : extraTextures) {
				if (loader) {
					loader->RemoveTexture(*tex);
				}
				//~BSScaleformExternalTexture calls ReleaseTexture
			}
			extraTextures.clear();
		}

		class BookMenuCloseSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
		public:
			static BookMenuCloseSink* GetSingleton() {
				static BookMenuCloseSink singleton;
				return &singleton;
			}

			bool sinkAdded = false;
			
			RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
				RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
				if (!event){
					return RE::BSEventNotifyControl::kContinue;
				}
				
				if (IsBadReadPtr(event, sizeof(RE::MenuOpenCloseEvent))){
					return RE::BSEventNotifyControl::kContinue;
				}
				
				if (!event->opening && event->menuName == RE::BookMenu::MENU_NAME) {
					ReleaseAll();
					if (BookMenuCloseSink::GetSingleton()->sinkAdded){
						if (auto* ui = RE::UI::GetSingleton()) {
							ui->RemoveEventSink<RE::MenuOpenCloseEvent>(BookMenuCloseSink::GetSingleton());
						}
					}
				}
				return RE::BSEventNotifyControl::kContinue;
			}
		};

		//"...src='img://Textures/Interface/Books/x.png'..." -> "Textures/Interface/Books/x.png"
		std::vector<std::string> ExtractImagePaths(const std::string& a_text) {
			std::vector<std::string> paths;
			constexpr std::string_view kScheme = "img://";

			std::size_t pos = 0;
			while ((pos = a_text.find(kScheme, pos)) != std::string::npos) {
				pos += kScheme.size();
				const std::size_t end = a_text.find_first_of("'\" >", pos);
				if (end == std::string::npos) {
					break;
				}
				if (end > pos) {
					paths.emplace_back(a_text.substr(pos, end - pos));
				}
				pos = end;
			}
			return paths;
		}

		bool IsLoaded(RE::BookMenu* a_menu, const std::string& a_path) {
			for (auto& tex : a_menu->GetRuntimeData().bookTextures) {
				if (tex.filePath.c_str() && _stricmp(tex.filePath.c_str(), a_path.c_str()) == 0) {
					return true;
				}
			}
			for (auto& tex : extraTextures) {
				if (tex->filePath.c_str() && _stricmp(tex->filePath.c_str(), a_path.c_str()) == 0) {
					return true;
				}
			}
			return false;
		}

		//Call on the UI thread before SetBookText.
		void LoadImagesForText(RE::BookMenu* a_menu, const std::string& a_text) {
			auto* loader = GetImageLoader();
			if (!loader) {
				logger::warn("SetBookMenuText: no Scaleform image loader");
				return;
			}

			for (auto& path : ExtractImagePaths(a_text)) {
				if (IsLoaded(a_menu, path)) {
					continue;
				}

				auto tex = std::make_unique<RE::BSScaleformExternalTexture>();
				if (!tex->LoadPNG(RE::BSFixedString(path.c_str()))) {
					logger::warn("SetBookMenuText: failed to load image [{}]", path);
					continue;
				}

				loader->AddTexture(*tex);
				logger::trace("SetBookMenuText: loaded image [{}]", path);
				extraTextures.push_back(std::move(tex));
			}
			
			if (extraTextures.size() > 0){ //only need the event sink to listen for book menu closed if added extra textures
				if (!BookMenuCloseSink::GetSingleton()->sinkAdded){
					if (auto* ui = RE::UI::GetSingleton()) {
						ui->AddEventSink<RE::MenuOpenCloseEvent>(BookMenuCloseSink::GetSingleton());
					}
				}
			}
		}
	}
	
	//Set book text in opened book menu. Us SetFormDescription on book to keep the text permanent.
	bool SetBookMenuText(RE::StaticFunctionTag*, std::string text) {
		auto* ui = RE::UI::GetSingleton();
		if (!ui || !ui->IsMenuOpen(RE::BookMenu::MENU_NAME)) {
			logger::warn("Book Menu is not open");
			return false;
		}
		
		auto* tasks = SKSE::GetTaskInterface(); 
		if (!tasks) {
			return false;
		}

		tasks->AddUITask([text = std::move(text)]() {
			auto* ui = RE::UI::GetSingleton();
			RE::GPtr<RE::BookMenu> bookMenu = ui ? ui->GetMenu<RE::BookMenu>(RE::BookMenu::MENU_NAME) : nullptr;
			if (!bookMenu) {
				return;
			}

			auto* movie = bookMenu->GetRuntimeData().book.get();
			if (!movie) {
				return;
			}

			RE::GFxValue bookMc;
			if (!movie->GetVariable(&bookMc, "_root.BookMenu_mc") || !bookMc.IsDisplayObject()) {
				logger::warn("_root.BookMenu_mc not found");
				return;
			}

			bookImages::LoadImagesForText(bookMenu.get(), text);
			
			//1. stop a pagination pass that's still running, so it can't write into the new state.
			//   Normally it has already finished (CalculatePagination does every line in one call).
			RE::GFxValue paginationIndex;
			if (bookMc.GetMember("iPaginationIndex", &paginationIndex) &&
				paginationIndex.IsNumber() && paginationIndex.GetNumber() != -1) {
				RE::GFxValue arg;
				arg.SetNumber(paginationIndex.GetNumber());
				movie->Invoke("_global.clearInterval", nullptr, &arg, 1);
			}

			//2. remove the cached page clips -- they still show the old text
			RE::GFxValue pages;
			if (bookMc.GetMember("BookPages", &pages) && pages.IsArray()) {
				for (std::uint32_t i = 0; i < pages.GetArraySize(); i++) {
					RE::GFxValue page;
					if (pages.GetElement(i, &page) && page.IsDisplayObject()) {
						page.Invoke("removeMovieClip", nullptr, nullptr, 0);
					}
				}
			}

			//3. reset to what the constructor sets
			RE::GFxValue emptyPages, emptyPageInfo, zero;
			movie->CreateArray(&emptyPages);
			movie->CreateArray(&emptyPageInfo);
			zero.SetNumber(0);

			bookMc.SetMember("BookPages", emptyPages);
			bookMc.SetMember("PageInfoA", emptyPageInfo);
			bookMc.SetMember("iPageSetIndex", zero);
			bookMc.SetMember("iLeftPageNumber", zero);

			//4. new text -- notes and scrolls use a single page layout
			bool isNote = false;
			RE::GFxValue bNoteGfx;
			if (bookMc.GetMember("bNote", &bNoteGfx)){
				logger::info("bNote in book menu found");
				isNote = bNoteGfx.GetBool();
			}
			else {
				logger::info("bNote in book menu not found");
			}
				
			// if (auto* book = bookMenu->GetTargetForm()) {
			// 	logger::info("book in book menu found");
			// 	isNote = book->IsNote() || book->IsNoteScroll();
			// }
			// else {
			// 	logger::warn("book in book menu not found");
			// }

			RE::GFxValue args[2];
			args[0].SetString(text.c_str());
			args[1].SetBoolean(isNote);

			
			if (!bookMc.Invoke("SetBookText", nullptr, args, 2)) {
				logger::warn("SetBookText invoke failed");
				return;
			}

			//5. paginate now rather than waiting for SetBookText's 30ms interval. CalculatePagination
			//   processes every line, clears its own interval, then calls UpdatePages.
			bookMc.Invoke("CalculatePagination", nullptr, nullptr, 0);

			logger::trace("text replaced, isNote[{}]", isNote);
		});

		return true;
	}
	
    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) { 
        vm->RegisterFunction("GetUITargetType", "DbSkseFunctions", GetUiTargetType);
        vm->RegisterFunction("GetUITargetTypeAsString", "DbSkseFunctions", GetUiTargetTypeAsString);
        vm->RegisterFunction("GetUITargetValueAsString", "DbSkseFunctions", GetUiTargetValueAsString);
        vm->RegisterFunction("GetUiTargetMembers", "DbSkseFunctions", GetUiTargetMembers);
        vm->RegisterFunction("GetUiTargetMembersData", "DbSkseFunctions", GetUiTargetMembersData);
        vm->RegisterFunction("FindUiMember", "DbSkseFunctions", FindUiMember);
        vm->RegisterFunction("SetBookMenuText", "DbSkseFunctions", SetBookMenuText);
        return true;
    }
}