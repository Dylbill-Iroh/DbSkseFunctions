#pragma once

namespace gfuncs {
    /*RE::SkyrimVM* svm;
    RE::TESObjectREFR* lastPlayerActivatedRef;
    RE::TESObjectREFR* menuRef;
    RE::Actor* svPlayerRef;*/
    std::string uint32_to_string(uint32_t value);

    int GetRandomInt(int min, int max);

    std::string IntToHex(int i);

    int HexToInt(std::string hex);

    uint64_t StringToUint64_t(std::string s);

    //get the int after start char in string s. Example: GetIntAfterCharInString("arrows (21)") returns 21
    int GetIntAfterCharInString(std::string s, char startChar = '(', int iDefault = 0, int startIndex = 0);

    bool IsFormValid(RE::TESForm* form, bool checkDeleted = false);

    std::string GetFormEditorId(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string nullFormString);

    void SetFormName(RE::TESForm* baseForm, RE::BSFixedString nuName);

    RE::BSFixedString GetFormName(RE::TESForm* akForm, RE::BSFixedString nullString = "null", RE::BSFixedString noNameString = "", bool returnIdIfNull = true);

    std::string GetFormDataString(RE::TESForm* akForm, std::string nullString = "null", std::string noNameString = "No Name");

    RE::TESFile* GetFileForForm(RE::TESForm* akForm);

    void logFormMap(auto& map);

    void DelayedFunction(auto function, int delay) {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            function();
            });
        t.detach();
    }

    float timePointDiffToFloat(std::chrono::system_clock::time_point end, std::chrono::system_clock::time_point start);

    RE::VMHandle GetHandle(RE::TESForm* akForm);

    RE::VMHandle GetHandle(RE::BGSBaseAlias* akAlias);

    RE::VMHandle GetHandle(RE::ActiveEffect* akEffect);

    RE::TESObjectREFR* GetRefFromHandle(RE::RefHandle& handle);

    RE::TESObjectREFR* GetRefFromObjectRefHandle(RE::ObjectRefHandle refHandle);

    bool IsScriptAttachedToHandle(RE::VMHandle& handle, RE::BSFixedString& sScriptName);

    RE::Actor* GetPlayerDialogueTarget();

    void RefreshItemMenu();

    bool SetAliasQuestObjectFlag(RE::BGSBaseAlias* akAlias, bool set);

    bool IsAliasQuestObjectFlagSet(RE::BGSBaseAlias* akAlias);

    bool IsQuestObject(RE::TESObjectREFR* ref);

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    bool ContainerContainsRef(RE::TESObjectREFR* containerRef, RE::TESObjectREFR* ref);
    
    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    std::int32_t GetBaseFormCount(RE::TESObjectREFR* containerRef, RE::TESBoundObject* akForm);

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    std::int32_t GetItemCount(RE::TESObjectREFR* containerRef, RE::TESForm* item);

    template< typename T >
    std::string IntToHex(T i)
    {
        std::stringstream stream;
        stream << ""
            << std::setfill('0') << std::setw(sizeof(T) * 2)
            << std::hex << i;
        return stream.str();
    }

    template <class T>
    void RemoveFromVectorByValue(std::vector<T>& v, T value) {
        v.erase(std::remove(v.begin(), v.end(), value), v.end());
    }

    void String_ReplaceAll(std::string& s, std::string searchString, std::string replaceString);

    void String_ReplaceAll(std::string& s, std::vector<std::string> searchStrings, std::vector<std::string> replaceStrings);

    int GetIndexInVector(std::vector<RE::FormID>& v, RE::FormID& element);

    int GetIndexInVector(std::vector<RE::TESForm*>& v, RE::TESForm* element);

    int GetIndexInVector(std::vector<RE::BGSProjectile*>& v, RE::BGSProjectile* element);

    int GetIndexInVector(std::vector<RE::BSSoundHandle*>& v, RE::BSSoundHandle* element);

    int GetIndexInVector(std::vector<RE::BSSoundHandle> v, RE::BSSoundHandle element);

    int GetIndexInVector(std::vector<RE::VMHandle> v, RE::VMHandle& element);

    int GetIndexInVector(std::vector<RE::BSFixedString> v, RE::BSFixedString& element);

    int GetIndexInVector(std::vector<std::string> v, std::string& element);

    int GetIndexInVector(std::vector<RE::TESObjectREFR*> v, RE::TESObjectREFR* element);

    bool IsObjectInBSTArray(RE::BSTArray<RE::ObjectRefHandle>* v, RE::ObjectRefHandle& element);

    RE::NiAVObject* GetNiAVObjectForRef(RE::TESObjectREFR* ref);

    RE::BGSBaseAlias* GetQuestAliasById(RE::TESQuest* quest, int id);

    int GetWeatherType(RE::TESWeather* weather);

    RE::BSFixedString GetBSUIMessageDataTypeString(RE::BSUIMessageData* msgData);

    void RemoveDuplicates(std::vector<RE::VMHandle>& vec);

    void RemoveDuplicates(std::vector<RE::TESForm*>& vec);

    void CombineEventHandles(std::vector<RE::VMHandle>& handles, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& formHandles);

    void SendEvents(std::vector<RE::VMHandle> handles, RE::BSFixedString& sEvent, RE::BSScript::IFunctionArguments* args);

    void Install();
}
