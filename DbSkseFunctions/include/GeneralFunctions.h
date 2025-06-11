#pragma once

namespace gfuncs {
    extern std::chrono::system_clock::time_point lastTimeGameWasLoaded;

    void LogAndMessage(std::string message, int logLevel = 0, int debugLevel = 0);

    void ConvertToLowerCase(std::string& s);

    std::string uint32_to_string(uint32_t value);

    int GetRandomInt(int min, int max);

    bool IsHexString(std::string s);

    bool IsDecString(std::string s);

    std::string IntToHex(int i);

    std::string IntToHexPapyrus(int i);

    int HexToInt(std::string hex);

    uint64_t StringToUint64_t(std::string s);

    bool StringHasNCharacters(std::string s, char c, int minNumber);

    //get the int after start char in string s. Example: GetIntAfterCharInString("arrows (21)") returns 21
    int GetIntAfterCharInString(std::string s, char startChar = '(', int iDefault = 0, int startIndex = 0);

    bool IsFormValid(RE::TESForm* form, bool checkDeleted = false);

    bool AllFormsValid(std::vector<RE::TESForm*> forms, std::vector<std::string> formParamNames, std::string callingFunctionName, bool checkDeleted = false);

    std::string GetFormEditorId(RE::StaticFunctionTag*, RE::TESForm* akForm, std::string nullFormString);

    void SetFormName(RE::TESForm* baseForm, RE::BSFixedString nuName);

    RE::BSFixedString GetFormName(RE::TESForm* akForm, RE::BSFixedString nullString = "null", RE::BSFixedString noNameString = "", bool returnIdIfNull = true);
    
    std::string GetFormNameAndId(RE::TESForm* akForm, std::string nullString = "null", std::string noNameString = "");

    std::string GetFormDataString(RE::TESForm* akForm, std::string nullString = "null", std::string noNameString = "No Name");

    RE::TESFile* GetFileForForm(RE::TESForm* akForm);

    RE::TESFile* GetFileForRawFormId(RE::FormID rawFormID, RE::TESFile* file);

    std::string GetModName(RE::TESForm* form);

    void logFormMap(auto& map);

    void DelayedFunction(auto function, int delay) {
        std::thread t([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            function();
            });
        t.detach();
    }

    //return difference of time points in seconds as float
    float timePointDiffToFloat(std::chrono::system_clock::time_point end, std::chrono::system_clock::time_point start);

    RE::VMHandle GetHandle(RE::TESForm* akForm);

    RE::VMHandle GetHandle(RE::BGSBaseAlias* akAlias);

    RE::VMHandle GetHandle(RE::ActiveEffect* akEffect);

    RE::ActiveEffect* GetActiveEffectFromHandle(RE::VMHandle handle);

    RE::TESObjectREFR* GetRefFromHandle(RE::RefHandle& handle);

    RE::TESObjectREFR* GetRefFromObjectRefHandle(RE::ObjectRefHandle refHandle);

    bool IsScriptAttachedToHandle(RE::VMHandle& handle, RE::BSFixedString& sScriptName);

    bool IsScriptAttachedToRef(RE::TESObjectREFR* ref, RE::BSFixedString sScriptName);

    bool IsScriptAttachedToForm(RE::TESForm* akForm, RE::BSFixedString sScriptName);

    RE::BSScript::Object* GetAttachedScriptObject(RE::VMHandle& handle, RE::BSFixedString& sScriptName);

    RE::TESObjectREFR* GetDialogueTarget(RE::Actor* actor);

    RE::Actor* GetPlayerDialogueTarget();

    void RefreshItemMenu();

    bool IsRefActivatedMenu(RE::BSFixedString menu);

    bool IsRefActivatedMenuOpen();

    bool SetAliasQuestObjectFlag(RE::BGSBaseAlias* akAlias, bool set);

    bool IsAliasQuestObjectFlagSet(RE::BGSBaseAlias* akAlias);

    bool IsQuestObject(RE::TESObjectREFR* ref);

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    bool ContainerContainsRef(RE::TESObjectREFR* containerRef, RE::TESObjectREFR* ref);
    
    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    std::int32_t GetBaseFormCount(RE::TESObjectREFR* containerRef, RE::TESBoundObject* akForm);

    //Thanks to Meridiano, author of Papyrus Ini Manipulator for this.
    std::int32_t GetItemCount(RE::TESObjectREFR* containerRef, RE::TESForm* item);

    bool formIsBowOrCrossbow(RE::TESForm* akForm);

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

    bool StringContainsStringInVector(std::vector<std::string>& v, std::string value);

    int GetIndexInVector(std::vector<uint32_t>& v, uint32_t& element);

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

    void RemoveDuplicates(std::vector<std::string>& vec);

    void RemoveDuplicates(std::vector<RE::FormID>& vec);

    void RemoveDuplicates(std::vector<RE::VMHandle>& vec);

    void RemoveDuplicates(std::vector<RE::TESForm*>& vec);

    void CombineEventHandles(std::vector<RE::VMHandle>& handles, RE::TESForm* akForm, std::map<RE::TESForm*, std::vector<RE::VMHandle>>& formHandles);

    void SendEvents(std::vector<RE::VMHandle> handles, RE::BSFixedString& sEvent, RE::BSScript::IFunctionArguments* args);
    
    void SendEvents(std::vector<RE::VMHandle> handles, const RE::BSFixedString& sEvent, RE::BSScript::IFunctionArguments* args);
    
    RE::TESForm* FindNullForm();

    void Install();
}
