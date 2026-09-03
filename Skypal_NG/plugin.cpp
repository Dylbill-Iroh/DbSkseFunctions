#include <Windows.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>
#include <chrono>
#include "RE/C/CFilter.h"
#include "mini/ini.h"

std::chrono::steady_clock::time_point pluginStartTimePoint;

namespace logger = SKSE::log;

void SetupLog() {
    // first, create a file instance
    mINI::INIFile file("Data/SKSE/Plugins/doticu_skypal.ini");

    // next, create a structure that will hold data
    mINI::INIStructure ini;

    // now we can read the file
    file.read(ini);

    std::string akLevel = ini["LOG"]["iMinLevel"];
    spdlog::level::level_enum iLevel = static_cast<spdlog::level::level_enum>(std::stoi(akLevel));

    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::level_enum::trace);
    spdlog::flush_on(spdlog::level::trace);
    // logger::trace("{} level set to {}", "SetupLog", iLevel);
    spdlog::set_level(iLevel);
} 

template< typename T >
std::string IntToHex(T i)
{
    std::stringstream stream;
    stream << ""
        << std::setfill('0') << std::setw(sizeof(T) * 2)
        << std::hex << i;
    return stream.str();
}

RE::BSFixedString GetFormName(RE::TESForm* akForm, RE::BSFixedString nullString = "null", RE::BSFixedString noNameString = "", bool returnIdIfNull = false) {
    if (!akForm) {
        return nullString;
    }

    RE::TESObjectREFR* ref = akForm->As<RE::TESObjectREFR>();
    RE::BSFixedString name;
    if (ref) {
        name = ref->GetDisplayFullName();
        if (name == "") {
            name = ref->GetBaseObject()->GetName();
        }
    }
    else {
        name = akForm->GetName();
    }

    if (name == "") {
        if (returnIdIfNull) {
            name = IntToHex(akForm->GetFormID());
        }
        else {
            name = noNameString;
        }
    }
    return name;
}

int GetIndexInVector(std::vector<int>& v, int element) {
    if (v.size() == 0) {
        return -1;
    }

    int m = v.size();
    for (int i = 0; i < m; i++) {
        if (v[i] == element) {
            return i;
        }
    }

    return -1;
}

static inline RE::ObjectRefHandle GetSelectedRefHandle() {
    REL::Relocation<RE::ObjectRefHandle*> selectedRef{ RELOCATION_ID(519394, REL::Module::get().version().patch() < 1130 ? 405935 : 504099) };
    return *selectedRef;
}

static inline RE::NiPointer<RE::TESObjectREFR> GetSelectedRef() {
    auto handle = GetSelectedRefHandle();
    return handle.get();
}

//edited form ConsoleUtil NG
static inline void ExecuteConsoleCommand(std::string a_command, RE::TESObjectREFR* objRef) {
    const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
	const auto script = scriptFactory ? scriptFactory->Create() : nullptr;
	if (script) {
		script->SetCommand(a_command); 
		RE::TESObjectREFR* ref = nullptr;
		
		if (objRef) {
			ref = objRef;
		} 
		else {
			const auto selectedRef = RE::Console::GetSelectedRef();
			if (selectedRef) {
				ref = selectedRef.get();
				if (!ref) {
					ref = nullptr;
				}
			}
		}
		
		script->CompileAndRun(ref);
		delete script;
	}
}

std::vector<RE::TESObjectREFR*> All(RE::StaticFunctionTag*) {
    std::vector<RE::TESObjectREFR*> refs;
    const auto& [allForms, lock] = RE::TESForm::GetAllForms();
    for (auto& [id, form] : *allForms) {
        auto* ref = form->AsReference();
        if (ref) {
            refs.push_back(ref);
        }
    }

    return refs;
}

std::vector<RE::TESObjectREFR*> Grid(RE::StaticFunctionTag*) {
    std::vector<RE::TESObjectREFR*> refs;
    const auto& [allForms, lock] = RE::TESForm::GetAllForms();
    for (auto& [id, form] : *allForms) {
        auto* ref = form->AsReference();
        if (ref) {
            auto* cell = ref->GetParentCell();
            if (cell) {
                if (cell->IsAttached()) {
                    refs.push_back(ref);
                }
            }
        }
    }

    return refs;
}

std::vector<RE::TESObjectREFR*> All_Filter_Bases(RE::StaticFunctionTag*, std::vector<RE::TESForm*> bases, std::string mode) {
    std::vector<RE::TESObjectREFR*> refs;
    const auto& [allForms, lock] = RE::TESForm::GetAllForms();
    if (mode == "!") {
        for (auto& [id, form] : *allForms) {
            auto* ref = form->AsReference();
            if (ref) {
                if (std::find(bases.begin(), bases.end(), ref->GetBaseObject()) == bases.end()) {
                    refs.push_back(ref);
                }
            }
        }
    }
    else {
        for (auto& [id, form] : *allForms) {
            auto* ref = form->AsReference();
            if (ref) {
                if (std::find(bases.begin(), bases.end(), ref->GetBaseObject()) != bases.end()) {
                    refs.push_back(ref);
                }
            }
        }
    }

    return refs;
}

std::vector<RE::TESObjectREFR*> All_Filter_Bases_Form_List(RE::StaticFunctionTag*, RE::BGSListForm* akFormlist, std::string mode) {
    std::vector<RE::TESObjectREFR*> refs;
    const auto& [allForms, lock] = RE::TESForm::GetAllForms();
    if (mode == "!") {
        for (auto& [id, form] : *allForms) {
            auto* ref = form->AsReference();
            if (ref) {
                if (!akFormlist->HasForm(ref->GetBaseObject())) {
                    refs.push_back(ref);
                }
            }
        }
    }
    else {
        for (auto& [id, form] : *allForms) {
            auto* ref = form->AsReference();
            if (ref) {
                if (akFormlist->HasForm(ref->GetBaseObject())) {
                    refs.push_back(ref);
                }
            }
        }
    }

    return refs;
}

std::vector<RE::TESObjectREFR*> Grid_Filter_Bases(RE::StaticFunctionTag*, std::vector<RE::TESForm*> bases, std::string mode) {
    std::vector<RE::TESObjectREFR*> refs;

    auto* tes = RE::TES::GetSingleton();
    if (!tes) {
        logger::error("{} couldn't get TES singleton", __func__);
        return refs;
    }

    if (mode == "!") {
		const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		for (auto& [id, form] : *allForms) {
			auto* ref = form->AsReference();
			if (ref) {
				auto* cell = ref->GetParentCell(); 
				if (cell){
					if (cell->IsAttached()){
						if (std::find(bases.begin(), bases.end(), ref->GetBaseObject()) == bases.end()) {
							refs.push_back(ref);
						}
					}
				}
			}
		}
    }
    else {
		const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		for (auto& [id, form] : *allForms) {
			auto* ref = form->AsReference();
			if (ref) {
				auto* cell = ref->GetParentCell(); 
				if (cell){
					if (cell->IsAttached()){
						if (std::find(bases.begin(), bases.end(), ref->GetBaseObject()) != bases.end()) {
							refs.push_back(ref);
						}
					}
				}
			}
		}
    }

    return refs;
}

std::vector<RE::TESObjectREFR*> Grid_Filter_Bases_Form_List(RE::StaticFunctionTag*, RE::BGSListForm* bases, std::string mode) {
    std::vector<RE::TESObjectREFR*> refs;

    auto* tes = RE::TES::GetSingleton();
    if (!tes) {
        logger::error("{} couldn't get TES singleton", __func__);
        return refs;
    }

    if (mode == "!") {
		const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		for (auto& [id, form] : *allForms) {
			auto* ref = form->AsReference();
			if (ref) {
				auto* cell = ref->GetParentCell(); 
				if (cell){
					if (cell->IsAttached()){
						if (!bases->HasForm(ref->GetBaseObject())) {
							refs.push_back(ref);
						}
					}
				}
			}
		} 
    }
    else {
		const auto& [allForms, lock] = RE::TESForm::GetAllForms();
		for (auto& [id, form] : *allForms) {
			auto* ref = form->AsReference();
			if (ref) {
				auto* cell = ref->GetParentCell(); 
				if (cell){
					if (cell->IsAttached()){
						if (bases->HasForm(ref->GetBaseObject())) {
							refs.push_back(ref);
						}
					}
				}
			}
		}  
    }

    return refs;
}

int Count_Disabled(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs) {
    int count = 0;
    for (int i = 0; i < refs.size(); i++) {
        if (refs[i]) {
            if (refs[i]->IsDisabled()) {
                count += 1;
            }
        }
    }
    return count;
}

int Count_Enabled(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs) {
    int count = 0;
    for (int i = 0; i < refs.size(); i++) {
        if (refs[i]) {
            if (!refs[i]->IsDisabled()) {
                count += 1;
            }
        }
    }
    return count;
}

void Disable(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs) {
    for (int i = 0; i < refs.size(); i++) {
        if (refs[i]) {
            refs[i]->Disable();
        }
    }
}

void Enable(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs) {
    for (int i = 0; i < refs.size(); i++) {
        if (refs[i]) {
            //didn't see an Enable function in TESObjectREFR, so using console command as workaround.
            ExecuteConsoleCommand("enable", refs[i]);
        }
    }
}

std::vector<RE::TESObjectREFR*> Filter_Base_Form_Types(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::vector<int> formTypes, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in", __func__);
        return returnRefs;
    }

    if (formTypes.size() == 0) {
        logger::warn("{} no formTypes passed in", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                int formType = static_cast<int>(refs[i]->GetBaseObject()->GetFormType());
                if (GetIndexInVector(formTypes, formType) == -1) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                int formType = static_cast<int>(refs[i]->GetBaseObject()->GetFormType());
                if (GetIndexInVector(formTypes, formType) != -1) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }

    return returnRefs;
}

std::vector<RE::TESObjectREFR*> Filter_Bases(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::vector<RE::TESForm*> bases, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in", __func__);
        return returnRefs;
    }

    if (bases.size() == 0) {
        logger::warn("{} no bases passed in", __func__);
        return returnRefs;
    }


    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (std::find(bases.begin(), bases.end(), refs[i]->GetBaseObject()) == bases.end()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (std::find(bases.begin(), bases.end(), refs[i]->GetBaseObject()) != bases.end()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }

    return returnRefs;
}

int GetFormlistSize(RE::BGSListForm* akFormList) {
    if (!akFormList) {
        return 0;
    }
    return(akFormList->forms.size() + akFormList->scriptAddedFormCount);
}

std::vector<RE::TESObjectREFR*> Filter_Bases_Form_List(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, RE::BGSListForm* akFormlist, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in", __func__);
        return returnRefs;
    }

    if (GetFormlistSize(akFormlist) == 0) {
        logger::warn("{} no bases passed in", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (!akFormlist->HasForm(refs[i]->GetBaseObject())) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (akFormlist->HasForm(refs[i]->GetBaseObject())) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }

    return returnRefs;
}

int GetCollisionLayerType(RE::TESObjectREFR* ref) {
    if (!ref) {
        //logger::warn("{} ref doesn't exist", __func__);
        return 0;
    }

    int type = 0;

    auto* nav = ref->Get3D();
    if (!nav) {
        //logger::warn("{} 3d for ref {} Id {:x} not found", __func__, GetFormName(ref), ref->GetFormID());
        return 0;
    }

    return static_cast<int>(nav->GetCollisionLayer());

}

void SetCollisionLayerType(RE::TESObjectREFR* ref, int collision_layer_type) {
    logger::info("{} called {}", __func__, collision_layer_type);

    if (!ref) {
        //logger::warn("{} ref doesn't exist", __func__);
        return;
    }

    if (collision_layer_type < 1 || collision_layer_type > 46) {
        logger::error("{} collision_layer_type {} is invalid", __func__, collision_layer_type);
        return;
    }

    RE::NiAVObject* nav = ref->Get3D();
    if (!nav) {
        logger::warn("{} 3d for ref {} Id {:x} not found", __func__, GetFormName(ref), ref->GetFormID());
        return;
    }

    logger::info("{} current collision layer is {}", __func__, int(nav->GetCollisionLayer()));

    RE::Actor* akActor = ref->As<RE::Actor>(); 
    if (akActor) {
        RE::COL_LAYER coLayer = static_cast<RE::COL_LAYER>(collision_layer_type);
        std::string node = "";
        SKSE::GetTaskInterface()->AddTask([node, nav, coLayer, akActor]() {
            RE::CFilter filter;
            akActor->GetCollisionFilterInfo(filter);
            nav->SetCollisionLayerAndGroup(coLayer, filter.filter);
            nav->UpdateRigidConstraints(true);
            logger::info("collision layer set on actor");
            });
    }
    else {
        RE::COL_LAYER coLayer = static_cast<RE::COL_LAYER>(collision_layer_type);
        std::string node = "";
        SKSE::GetTaskInterface()->AddTask([node, nav, coLayer]() {
            nav->SetCollisionLayer(coLayer);
            logger::info("collision layer set");
            //nav->SetCollisionLayerAndGroup(coLayer, filterInfo + 1);
            //nav->UpdateRigidConstraints(true);
            });
    }
}

std::vector<RE::TESObjectREFR*> Filter_Collision_Layer_Types(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::vector<int> collision_layer_types, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in", __func__);
        return returnRefs;
    }

    if (collision_layer_types.size() == 0) {
        logger::warn("{} no collision_layer_types passed in", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                int type = GetCollisionLayerType(refs[i]);
                if (std::find(collision_layer_types.begin(), collision_layer_types.end(), type) == collision_layer_types.end()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                int type = GetCollisionLayerType(refs[i]);
                if (std::find(collision_layer_types.begin(), collision_layer_types.end(), type) != collision_layer_types.end()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    return returnRefs;
}

void Change_Collision_Layer_Type(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, int collision_layer_type) {
    if (collision_layer_type < 1 || collision_layer_type > 46) {
        logger::error("{} collision_layer_type {} is invalid", __func__, collision_layer_type);
        return;
    }

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in", __func__);
        return;
    }

    RE::COL_LAYER akLayer = static_cast<RE::COL_LAYER>(collision_layer_type);

    for (int i = 0; i < refsSize; i++) {
        if (refs[i]) {
            SetCollisionLayerType(refs[i], collision_layer_type);
        }
    }
}

std::vector<RE::TESObjectREFR*> Filter_Deleted(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (!refs[i]->IsDeleted()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (refs[i]->IsDeleted()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }

    return returnRefs;
}

std::vector<RE::TESObjectREFR*> Filter_Distance(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, float distance, RE::TESObjectREFR* from, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;
    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in", __func__);
        return returnRefs;
    }

    if (!from) {
        from = RE::TESForm::LookupByID<RE::TESForm>(20)->As<RE::TESObjectREFR>();
    }

    if (!from) {
        logger::error("{} from ref is none and couldn't find playerRef", __func__);
        return returnRefs;
    }

    if (distance < 0.0) {
        distance = 0.0;
    }

    if (mode == ">") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (refs[i]->GetPosition().GetDistance(from->GetPosition()) > distance) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (refs[i]->GetPosition().GetDistance(from->GetPosition()) < distance) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }

    return returnRefs;
}

std::vector<RE::TESObjectREFR*> Filter_Enabled(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (refs[i]->IsDisabled()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (!refs[i]->IsDisabled()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }

    return returnRefs;
}

std::vector<RE::TESObjectREFR*> Filter_3dLoaded(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (!refs[i]->Is3DLoaded()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (refs[i]->Is3DLoaded()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }

    return returnRefs;
}

std::vector<RE::TESObjectREFR*> Filter_Form_Types(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::vector<int> formTypes, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in", __func__);
        return returnRefs;
    }

    if (formTypes.size() == 0) {
        logger::warn("{} no formTypes passed in", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                int formType = static_cast<int>(refs[i]->GetBaseObject()->GetFormType());
                if (GetIndexInVector(formTypes, formType) == -1) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                int formType = static_cast<int>(refs[i]->GetBaseObject()->GetFormType());
                if (GetIndexInVector(formTypes, formType) != -1) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }

    return returnRefs;
}

int CountNumberOfKeywordsRefHas(RE::StaticFunctionTag* tag, RE::TESObjectREFR* ref, std::vector<RE::BGSKeyword*> keywords) {
    int count = 0;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return count;
    }

    int size = keywords.size();

    if (size == 0) {
        logger::warn("{} no keywords passed in", __func__);
        return count;
    }

    for (int i = 0; i < size; i++) {
        if (keywords[i]) {
            std::vector<RE::BGSKeyword*> keyword {keywords[i]};
            //hasKeyword function is missing, using this function as workaround
            if (ref->HasKeywordInArray(keyword, true)) {
                count += 1;
            }
        }
    }
    return count;
}

int CountNumberOfKeywordsRefHas_cpp(RE::TESObjectREFR* ref, std::vector<RE::BGSKeyword*> keywords) {
    int count = 0;
    if (!ref) {
        return count;
    }

    int size = keywords.size();

    if (size == 0) {
        logger::warn("{} no keywords passed in", __func__);
        return count;
    }

    for (int i = 0; i < size; i++) {
        if (keywords[i]) {
            std::vector<RE::BGSKeyword*> keyword {keywords[i]};
            //hasKeyword function is missing, using this function as workaround
            if (ref->HasKeywordInArray(keyword, true)) {
                count += 1;
            }
        }
    }
    return count;
}

bool refHasAtLeastOneKeyword(RE::StaticFunctionTag* tag, RE::TESObjectREFR* ref, std::vector<RE::BGSKeyword*> keywords) {
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return false;
    }

    int size = keywords.size();

    if (size == 0) {
        logger::warn("{} no keywords passed in", __func__);
        return false;
    }

    return ref->HasKeywordInArray(keywords, false);
}

bool refHasAtLeastOneKeyword_cpp(RE::TESObjectREFR* ref, std::vector<RE::BGSKeyword*> keywords) {
    if (!ref) {
        return false;
    }
    return ref->HasKeywordInArray(keywords, false);
}

//fix
std::vector<RE::BGSKeyword*> filter_keywordsOnRef(RE::StaticFunctionTag* tag, RE::TESObjectREFR* ref, std::vector<RE::BGSKeyword*> keywords, std::string mode) {
    std::vector<RE::BGSKeyword*> returnKeywords;
    if (!ref) {
        logger::warn("{} ref doesn't exist", __func__);
        return returnKeywords;
    }

    int size = keywords.size();

    if (size == 0) {
        logger::warn("{} no keywords passed in", __func__);
        return returnKeywords;
    }

    if (mode == "!") {
        for (int i = 0; i < size; i++) {
            if (keywords[i]) {
                std::vector<RE::BGSKeyword*> keyword {keywords[i]};
                //hasKeyword function is missing, using this function as workaround
                if (!ref->HasKeywordInArray(keyword, true)) {
                    returnKeywords.push_back(keywords[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            if (keywords[i]) {
                std::vector<RE::BGSKeyword*> keyword {keywords[i]};
                //hasKeyword function is missing, using this function as workaround
                if (ref->HasKeywordInArray(keyword, true)) {
                    returnKeywords.push_back(keywords[i]);
                }
            }
        }
    }

    return returnKeywords;
}

// if (keywords == none) : Passes an empty array.
// if (mode == "|") : Passes refs that match any keyword. (OR Gate) //default
// if (mode == "&") : Passes refs that match all keywords. (AND Gate)
// if (mode == "^") : Passes refs that match exactly one keyword. (XOR Gate)
// if (mode == "!|") : Passes refs that match no keywords. (NOR Gate)
// if (mode == "!&") : Passes refs that do not match all of the keywords. (NAND Gate)
// if (mode == "!^") : Passes refs that match 0 or more than 1 keyword. (XNOR Gate)
std::vector<RE::TESObjectREFR*> Filter_Keywords(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::vector<RE::BGSKeyword*> keywords, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in", __func__);
        return returnRefs;
    }

    if (keywords.size() == 0) {
        logger::warn("{} no keywords passed in", __func__);
        return returnRefs;
    }

    if (mode == "&") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (refs[i]->HasKeywordInArray(keywords, true)) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "^") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (CountNumberOfKeywordsRefHas_cpp(refs[i], keywords) == 1) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "!|") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (!refs[i]->HasKeywordInArray(keywords, false)) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "!&") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (!refs[i]->HasKeywordInArray(keywords, true)) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "!^") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                int count = CountNumberOfKeywordsRefHas_cpp(refs[i], keywords);
                if (count == 0 || count > 1) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (refs[i]->HasKeywordInArray(keywords, false)) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    return returnRefs;
}

int GetFactionRank(RE::Actor* akActor, RE::TESFaction* faction) {
    int returnRank = -2;
    if (!akActor) {
        return returnRank;
    }
    if (!faction) {
        return returnRank;
    }
    akActor->VisitFactions([&](RE::TESFaction* akfaction, int8_t rank) -> bool {
        if (akfaction == faction) {
            returnRank = rank;
            return true;
        }
        return true;
        });
    return returnRank;
}

bool IsActorOwnerOfEncounterZone_cpp(RE::Actor* akActor, RE::BGSEncounterZone* encounterZone, bool& hasOwner) {
    if (!akActor) {
        //logger::warn("{} akActor doesn't exist", __func__);
        return false;
    }
    if (!encounterZone) {
        //logger::warn("{} encounterZone doesn't exist", __func__);
        return false;
    }

    RE::TESForm* ownerForm = encounterZone->data.zoneOwner;

    if (ownerForm) {
        hasOwner = true;
        RE::TESFaction* ownerFaction = ownerForm->As<RE::TESFaction>();

        if (ownerFaction) {
            //logger::info("{} owner {} ID {:x} is a faction", __func__, GetFormName(ownerFaction), ownerFaction->GetFormID());
            return (GetFactionRank(akActor, ownerFaction) >= encounterZone->data.ownerRank);
        }

        RE::TESNPC* ownerNpc = ownerForm->As<RE::TESNPC>();
        if (ownerNpc) {
            //logger::info("{} owner {} ID {:x} is an npc", __func__, GetFormName(ownerNpc), ownerNpc->GetFormID());
            return (ownerNpc == akActor->GetBaseObject());
        }
    }
    return false;
}

bool ActorIsOwnerOfRef(RE::StaticFunctionTag*, RE::TESObjectREFR* akRef, RE::Actor* akActor) {
    if (!akRef) {
        logger::warn("{} akRef doesn't exist", __func__);
        return false;
    }

    if (!akActor) {
        logger::warn("{} akActor doesn't exist", __func__);
        return false;
    }

    bool hasOwner;
    auto* ownerBase = akActor->GetActorBase();
    auto* refActorOwner = akRef->GetActorOwner();

    if (refActorOwner) {
        if (refActorOwner == ownerBase) {
            return true;
        }
    }

    //logger::info("{} {}: getting ref factionOwner", __func__, i);

    auto* factionOwner = akRef->GetFactionOwner();
    if (factionOwner) {
        if (GetFactionRank(akActor, factionOwner) > -1) {
            return true;
        }
    }

    //logger::info("{} {}: getting akRef encounter zone", __func__, i);

    RE::BGSEncounterZone* encounterZone = akRef->extraList.GetEncounterZone();
    if (IsActorOwnerOfEncounterZone_cpp(akActor, encounterZone, hasOwner)) {
        return true;
    }

    //logger::info("{} {}: getting cell", __func__, i);

    auto* cell = akRef->GetParentCell();
    if (cell) {
        auto* cellOwner = cell->GetActorOwner();
        if (cellOwner) {
            if (cellOwner == ownerBase) {
                return true;
            }
        }

        //logger::info("{} {}: getting cellFactionOwner", __func__, i);

        auto* cellFactionOwner = cell->GetFactionOwner();
        if (cellFactionOwner) {
            if (GetFactionRank(akActor, cellFactionOwner) > -1) {
                return true;
            }
        }

        //::info("{} {}: getting cell encounterZone", __func__, i);

        auto* cellEncounterZone = cell->extraList.GetEncounterZone();
        if (IsActorOwnerOfEncounterZone_cpp(akActor, cellEncounterZone, hasOwner)) {
            return true;
        }
    }
    //logger::info("{} {}: getting worldSpace", __func__, i);

    auto* worldSpace = akRef->GetWorldspace();
    if (worldSpace) {

        //logger::info("{} {}: getting worldEncounterZone", __func__, i);

        auto* worldEncounterZone = worldSpace->encounterZone;
        if (IsActorOwnerOfEncounterZone_cpp(akActor, worldEncounterZone, hasOwner)) {
            return true;
        }
    }
    return false;
}

bool ActorIsOwnerOfRef_cpp(RE::TESObjectREFR* akRef, RE::Actor* akActor) {
    if (!akRef) {
        //logger::warn("{} akRef doesn't exist", __func__);
        return false;
    }

    if (!akActor) {
        //logger::warn("{} akActor doesn't exist", __func__);
        return false;
    }

    bool hasOwner;
    auto* ownerBase = akActor->GetActorBase();
    auto* refActorOwner = akRef->GetActorOwner();

    if (refActorOwner) {
        if (refActorOwner == ownerBase) {
            return true;
        }
    }

    //logger::info("{} {}: getting ref factionOwner", __func__, i);

    auto* factionOwner = akRef->GetFactionOwner();
    if (factionOwner) {
        if (GetFactionRank(akActor, factionOwner) > -1) {
            return true;
        }
    }

    //logger::info("{} {}: getting akRef encounter zone", __func__, i);

    RE::BGSEncounterZone* encounterZone = akRef->extraList.GetEncounterZone();
    if (IsActorOwnerOfEncounterZone_cpp(akActor, encounterZone, hasOwner)) {
        return true;
    }

    //logger::info("{} {}: getting cell", __func__, i);

    auto* cell = akRef->GetParentCell();
    if (cell) {
        auto* cellOwner = cell->GetActorOwner();
        if (cellOwner) {
            if (cellOwner == ownerBase) {
                return true;
            }
        }

        //logger::info("{} {}: getting cellFactionOwner", __func__, i);

        auto* cellFactionOwner = cell->GetFactionOwner();
        if (cellFactionOwner) {
            if (GetFactionRank(akActor, cellFactionOwner) > -1) {
                return true;
            }
        }

        //::info("{} {}: getting cell encounterZone", __func__, i);

        auto* cellEncounterZone = cell->extraList.GetEncounterZone();
        if (IsActorOwnerOfEncounterZone_cpp(akActor, cellEncounterZone, hasOwner)) {
            return true;
        }
    }
    //logger::info("{} {}: getting worldSpace", __func__, i);

    auto* worldSpace = akRef->GetWorldspace();
    if (worldSpace) {

        //logger::info("{} {}: getting worldEncounterZone", __func__, i);

        auto* worldEncounterZone = worldSpace->encounterZone;
        if (IsActorOwnerOfEncounterZone_cpp(akActor, worldEncounterZone, hasOwner)) {
            return true;
        }
    }
    return false;
}

int CountOwnersForRef(RE::StaticFunctionTag*, RE::TESObjectREFR* akRef, std::vector<RE::Actor*> owners) {
    int count = 0;
    if (!akRef) {
        logger::warn("{} akRef doesn't exist", __func__);
        return count;
    }

    int size = owners.size();
    if (size == 0) {
        logger::warn("{} no owners passed in", __func__);
        return count;
    }


    for (int i = 0; i < size; i++) {
        if (ActorIsOwnerOfRef_cpp(akRef, owners[i])) {
            count += 1;
        }
    }
    return count;
}

int CountOwnersForRef_cpp(RE::TESObjectREFR* akRef, std::vector<RE::Actor*> owners) {
    int count = 0;
    if (!akRef) {
        //logger::warn("{} akRef doesn't exist", __func__);
        return count;
    }

    int size = owners.size();
    if (size == 0) {
        //logger::warn("{} no owners passed in", __func__);
        return count;
    }


    for (int i = 0; i < size; i++) {
        if (ActorIsOwnerOfRef_cpp(akRef, owners[i])) {
            count += 1;
        }
    }
    return count;
}

bool RefHasAtLeastOneOwner(RE::StaticFunctionTag* tag, RE::TESObjectREFR* akRef, std::vector<RE::Actor*> owners) {
    if (!akRef) {
        logger::warn("{} akRef doesn't exist", __func__);
        return false;
    }

    int size = owners.size();
    if (size == 0) {
        logger::warn("{} no owners passed in", __func__);
        return false;
    }

    for (int i = 0; i < size; i++) {
        if (owners[i]) {
            if (ActorIsOwnerOfRef_cpp(akRef, owners[i])) {
                return true;
            }
        }
    }
    return false;
}

bool RefHasAtLeastOneOwner_cpp(RE::TESObjectREFR* akRef, std::vector<RE::Actor*> owners) {
    if (!akRef) {
        //logger::warn("{} akRef doesn't exist", __func__);
        return false;
    }

    int size = owners.size();
    if (size == 0) {
        //logger::warn("{} no owners passed in", __func__);
        return false;
    }

    for (int i = 0; i < size; i++) {
        if (owners[i]) {
            if (ActorIsOwnerOfRef_cpp(akRef, owners[i])) {
                return true;
            }
        }
    }
    return false;
}

std::vector<RE::Actor*> filter_OwnersOnRef(RE::StaticFunctionTag* tag, RE::TESObjectREFR* akRef, std::vector<RE::Actor*> owners, std::string mode) {
    std::vector<RE::Actor*> returnActors;
    if (!akRef) {
        logger::warn("{} akRef doesn't exist", __func__);
        return returnActors;
    }

    int size = owners.size();
    if (size == 0) {
        logger::warn("{} no owners passed in", __func__);
        return returnActors;
    }

    if (mode == "!") {
        for (int i = 0; i < size; i++) {
            if (owners[i]) {
                if (!ActorIsOwnerOfRef_cpp(akRef, owners[i])) {
                    returnActors.push_back(owners[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            if (owners[i]) {
                if (ActorIsOwnerOfRef_cpp(akRef, owners[i])) {
                    returnActors.push_back(owners[i]);
                }
            }
        }
    }
    return returnActors;
}

std::vector<RE::TESObjectREFR*> Filter_Owners(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::vector<RE::Actor*> owners, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in.", __func__);
        return returnRefs;
    }

    int owners_size = owners.size();
    if (owners_size == 0) {
        logger::warn("{} no owners passed in", __func__);
        return returnRefs;
    }

    if (mode == "&") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (CountOwnersForRef_cpp(refs[i], owners) == owners_size) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "^") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (CountOwnersForRef_cpp(refs[i], owners) == 1) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "!|") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (CountOwnersForRef_cpp(refs[i], owners) == 0) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "!&") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (CountOwnersForRef_cpp(refs[i], owners) < owners_size) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "!^") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                int count = CountOwnersForRef_cpp(refs[i], owners);
                if (count == 0 || count > 1) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        logger::info("{} | {}", __func__, refsSize);
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (RefHasAtLeastOneOwner_cpp(refs[i], owners)) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    return returnRefs;
}

bool ActorIsPotentialThiefOfRef(RE::StaticFunctionTag*, RE::TESObjectREFR* akRef, RE::Actor* akActor) {
    if (!akRef) {
        logger::warn("{} akRef doesn't exist", __func__);
        return false;
    }

    if (!akActor) {
        logger::warn("{} akActor doesn't exist", __func__);
        return false;
    }

    bool hasOwner = false;
    auto* ownerBase = akActor->GetActorBase();
    auto* refActorOwner = akRef->GetActorOwner();

    if (refActorOwner) {
        if (refActorOwner == ownerBase) {
            return false;
        }
        else {
            hasOwner = true;
        }
    }

    //logger::info("{} {}: getting ref factionOwner", __func__, i);

    auto* factionOwner = akRef->GetFactionOwner();
    if (factionOwner) {
        if (GetFactionRank(akActor, factionOwner) > -1) {
            return false;
        }
        else {
            hasOwner = true;
        }
    }

    //logger::info("{} {}: getting akRef encounter zone", __func__, i);

    RE::BGSEncounterZone* encounterZone = akRef->extraList.GetEncounterZone();
    if (IsActorOwnerOfEncounterZone_cpp(akActor, encounterZone, hasOwner)) {
        return false;
    }

    //logger::info("{} {}: getting cell", __func__, i);

    auto* cell = akRef->GetParentCell();
    if (cell) {
        auto* cellOwner = cell->GetActorOwner();
        if (cellOwner) {
            if (cellOwner == ownerBase) {
                return false;
            }
            else {
                hasOwner = true;
            }
        }

        //logger::info("{} {}: getting cellFactionOwner", __func__, i);

        auto* cellFactionOwner = cell->GetFactionOwner();
        if (cellFactionOwner) {
            if (GetFactionRank(akActor, cellFactionOwner) > -1) {
                return false;
            }
            else {
                hasOwner = true;
            }
        }

        //::info("{} {}: getting cell encounterZone", __func__, i);

        auto* cellEncounterZone = cell->extraList.GetEncounterZone();
        if (IsActorOwnerOfEncounterZone_cpp(akActor, cellEncounterZone, hasOwner)) {
            return false;
        }
    }
    //logger::info("{} {}: getting worldSpace", __func__, i);

    auto* worldSpace = akRef->GetWorldspace();
    if (worldSpace) {

        //logger::info("{} {}: getting worldEncounterZone", __func__, i);

        auto* worldEncounterZone = worldSpace->encounterZone;
        if (IsActorOwnerOfEncounterZone_cpp(akActor, worldEncounterZone, hasOwner)) {
            return false;
        }
    }
    if (hasOwner) {
        return true; //ref has owner and it is not the current owners[i] reference
    }
    return false;
}

bool ActorIsPotentialThiefOfRef_cpp(RE::TESObjectREFR* akRef, RE::Actor* akActor) {
    if (!akRef) {
        //logger::warn("{} akRef doesn't exist", __func__);
        return false;
    }

    if (!akActor) {
        //logger::warn("{} akActor doesn't exist", __func__);
        return false;
    }

    bool hasOwner = false;
    auto* ownerBase = akActor->GetActorBase();
    auto* refActorOwner = akRef->GetActorOwner();

    if (refActorOwner) {
        if (refActorOwner == ownerBase) {
            return false;
        }
        else {
            hasOwner = true;
        }
    }

    //logger::info("{} {}: getting ref factionOwner", __func__, i);

    auto* factionOwner = akRef->GetFactionOwner();
    if (factionOwner) {
        if (GetFactionRank(akActor, factionOwner) > -1) {
            return false;
        }
        else {
            hasOwner = true;
        }
    }

    //logger::info("{} {}: getting akRef encounter zone", __func__, i);

    RE::BGSEncounterZone* encounterZone = akRef->extraList.GetEncounterZone();
    if (IsActorOwnerOfEncounterZone_cpp(akActor, encounterZone, hasOwner)) {
        return false;
    }

    //logger::info("{} {}: getting cell", __func__, i);

    auto* cell = akRef->GetParentCell();
    if (cell) {
        auto* cellOwner = cell->GetActorOwner();
        if (cellOwner) {
            if (cellOwner == ownerBase) {
                return false;
            }
            else {
                hasOwner = true;
            }
        }

        //logger::info("{} {}: getting cellFactionOwner", __func__, i);

        auto* cellFactionOwner = cell->GetFactionOwner();
        if (cellFactionOwner) {
            if (GetFactionRank(akActor, cellFactionOwner) > -1) {
                return false;
            }
            else {
                hasOwner = true;
            }
        }

        //::info("{} {}: getting cell encounterZone", __func__, i);

        auto* cellEncounterZone = cell->extraList.GetEncounterZone();
        if (IsActorOwnerOfEncounterZone_cpp(akActor, cellEncounterZone, hasOwner)) {
            return false;
        }
    }
    //logger::info("{} {}: getting worldSpace", __func__, i);

    auto* worldSpace = akRef->GetWorldspace();
    if (worldSpace) {

        //logger::info("{} {}: getting worldEncounterZone", __func__, i);

        auto* worldEncounterZone = worldSpace->encounterZone;
        if (IsActorOwnerOfEncounterZone_cpp(akActor, worldEncounterZone, hasOwner)) {
            return false;
        }
    }
    if (hasOwner) {
        return true; //ref has owner and it is not the current owners[i] reference
    }
    return false;
}

int CountPotentialThievesForRef(RE::StaticFunctionTag* tag, RE::TESObjectREFR* akRef, std::vector<RE::Actor*> potential_thieves) {
    int count = 0;
    if (!akRef) {
        logger::warn("{} akRef doesn't exist", __func__);
        return count;
    }

    int size = potential_thieves.size();
    if (size == 0) {
        logger::warn("{} no potential_thieves passed in", __func__);
        return count;
    }

    for (int i = 0; i < size; i++) {
        if (ActorIsPotentialThiefOfRef_cpp(akRef, potential_thieves[i])) {
            count += 1;
        }
    }
    return count;
}

int CountPotentialThievesForRef_cpp(RE::TESObjectREFR* akRef, std::vector<RE::Actor*> potential_thieves) {
    int count = 0;
    if (!akRef) {
        //logger::warn("{} akRef doesn't exist", __func__);
        return count;
    }

    int size = potential_thieves.size();
    if (size == 0) {
        //logger::warn("{} no potential_thieves passed in", __func__);
        return count;
    }

    for (int i = 0; i < size; i++) {
        if (ActorIsPotentialThiefOfRef_cpp(akRef, potential_thieves[i])) {
            count += 1;
        }
    }
    return count;
}

bool RefHasAtLeastOnePotentialThief(RE::StaticFunctionTag* tag, RE::TESObjectREFR* akRef, std::vector<RE::Actor*> potential_thieves) {
    if (!akRef) {
        logger::warn("{} akRef doesn't exist", __func__);
        return false;
    }

    int size = potential_thieves.size();
    if (size == 0) {
        logger::warn("{} no potential_thieves passed in", __func__);
        return false;
    }

    for (int i = 0; i < size; i++) {
        if (ActorIsPotentialThiefOfRef_cpp(akRef, potential_thieves[i])) {
            return true;
        }
    }
    return false;
}

bool RefHasAtLeastOnePotentialThief_cpp(RE::TESObjectREFR* akRef, std::vector<RE::Actor*> potential_thieves) {
    if (!akRef) {
        logger::warn("{} akRef doesn't exist", __func__);
        //return false;
    }

    int size = potential_thieves.size();
    if (size == 0) {
        logger::warn("{} no potential_thieves passed in", __func__);
        //return false;
    }

    for (int i = 0; i < size; i++) {
        if (ActorIsPotentialThiefOfRef_cpp(akRef, potential_thieves[i])) {
            return true;
        }
    }
    return false;
}

std::vector<RE::Actor*> filter_PotentialThievesOnRef(RE::StaticFunctionTag* tag, RE::TESObjectREFR* akRef, std::vector<RE::Actor*> potential_thieves, std::string mode) {
    std::vector<RE::Actor*> returnActors;
    if (!akRef) {
        logger::warn("{} akRef doesn't exist", __func__);
        return returnActors;
    }

    int size = potential_thieves.size();
    if (size == 0) {
        logger::warn("{} no potential_thieves passed in", __func__);
        return returnActors;
    }

    if (mode == "!") {
        for (int i = 0; i < size; i++) {
            if (!ActorIsPotentialThiefOfRef_cpp(akRef, potential_thieves[i])) {
                returnActors.push_back(potential_thieves[i]);
            }
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            if (ActorIsPotentialThiefOfRef_cpp(akRef, potential_thieves[i])) {
                returnActors.push_back(potential_thieves[i]);
            }
        }
    }
    return returnActors;
}

std::vector<RE::TESObjectREFR*> Filter_Potential_Thieves(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::vector<RE::Actor*> potenital_thieves, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in.", __func__);
        return returnRefs;
    }

    int potenital_thieves_size = potenital_thieves.size();
    if (potenital_thieves_size == 0) {
        logger::warn("{} no potenital_thieves passed in", __func__);
        return returnRefs;
    }

    if (mode == "&") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (CountPotentialThievesForRef_cpp(refs[i], potenital_thieves) == potenital_thieves_size) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "^") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (CountPotentialThievesForRef_cpp(refs[i], potenital_thieves) == 1) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "!|") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (CountPotentialThievesForRef_cpp(refs[i], potenital_thieves) == 0) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "!&") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (CountPotentialThievesForRef_cpp(refs[i], potenital_thieves) < potenital_thieves_size) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else if (mode == "!^") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                int count = CountPotentialThievesForRef_cpp(refs[i], potenital_thieves);
                if (count == 0 || count > 1) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (RefHasAtLeastOnePotentialThief_cpp(refs[i], potenital_thieves)) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    return returnRefs;
}

std::vector<RE::TESObjectREFR*> Filter_WorldSpace(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, RE::TESWorldSpace* akWorldSpace, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in.", __func__);
        return returnRefs;
    }

    if (!akWorldSpace) {
        logger::warn("{} akWorldSpace doesn't exist", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refs.size(); i++) {
            if (refs[i]) {
                auto* worldSpace = refs[i]->GetWorldspace();
                if (worldSpace) {
                    if (worldSpace != akWorldSpace) {
                        returnRefs.push_back(refs[i]);
                    }
                }
                else {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refs.size(); i++) {
            auto* worldSpace = refs[i]->GetWorldspace();
            if (worldSpace) {
                if (worldSpace == akWorldSpace) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    return returnRefs;
}

std::vector<RE::TESObjectREFR*> Filter_OffLimits(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in.", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (!refs[i]->IsOffLimits()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (refs[i]->IsOffLimits()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    return returnRefs;
}

std::vector<RE::TESObjectREFR*> Filter_InventoryObjects(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in.", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (!refs[i]->IsInventoryObject()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (refs[i]->IsInventoryObject()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    return returnRefs;
}

std::vector<RE::TESObjectREFR*> Filter_PlayableObjects(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in.", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (!refs[i]->GetPlayable()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (refs[i]->GetPlayable()) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    return returnRefs;
}

bool RefIsAQuestObject(RE::TESObjectREFR* ref) {
    auto* refAliases = ref->extraList.GetByType<RE::ExtraAliasInstanceArray>();
    if (refAliases) {
        auto aliases = refAliases->aliases;
        for (int ii = 0; ii < aliases.size(); ii++) {
            if (aliases[ii]) {
                if (aliases[ii]->alias) {
                    if (aliases[ii]->alias->IsQuestObject()) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

std::vector<RE::TESObjectREFR*> Filter_QuestObjects(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in.", __func__);
        return returnRefs;
    }

    if (mode == "!") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (!RefIsAQuestObject(refs[i])) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                if (RefIsAQuestObject(refs[i])) {
                    returnRefs.push_back(refs[i]);
                }
            }
        }
    }
    return returnRefs;
}

std::vector<RE::TESObjectREFR*> Sort_Distance(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, RE::TESObjectREFR* from, std::string mode) {
    std::vector<RE::TESObjectREFR*> returnRefs;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in.", __func__);
        return returnRefs;
    }

    if (!from) {
        from = RE::TESForm::LookupByID<RE::TESForm>(20)->As<RE::TESObjectREFR>(); //playerRef
    }

    if (!from) {
        logger::error("{} from ref is none and couldn't find playerRef", __func__);
        return refs;
    }

    std::map<float, std::vector<RE::TESObjectREFR*>> refsMap;
    std::vector<float> distances;
    for (int i = 0; i < refsSize; i++) {
        if (refs[i]) {
            float distance = (refs[i]->GetPosition().GetDistance(from->GetPosition()));
            distances.push_back(distance);
            auto it = refsMap.find(distance);
            if (it == refsMap.end()) {
                std::vector<RE::TESObjectREFR*> distanceRefs;
                distanceRefs.push_back(refs[i]);
                refsMap[distance] = distanceRefs;
            }
            else {
                auto& distanceRefs = it->second;
                distanceRefs.push_back(refs[i]);
            }
        }
    }

    std::sort(distances.begin(), distances.end());
    if (mode == ">") {
        std::reverse(distances.begin(), distances.end());
    }
    distances.erase(std::unique(distances.begin(), distances.end()), distances.end());

    for (int i = 0; i < distances.size(); i++) {
        auto it = refsMap.find(distances[i]);
        if (it != refsMap.end()) {
            auto& v = it->second;
            for (auto* akRef : v) {
                returnRefs.push_back(akRef);
            }
        }
    }

    return returnRefs;
}

std::vector<RE::TESForm*> From_References(RE::StaticFunctionTag*, std::vector<RE::TESObjectREFR*> refs, std::string mode) {
    std::vector<RE::TESForm*> returnForms;

    int refsSize = refs.size();
    if (refsSize == 0) {
        logger::warn("{} no refs passed in.", __func__);
        return returnForms;
    }

    if (mode == "...") {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                RE::TESForm* akForm = refs[i]->GetBaseObject();
                if (akForm) {
                    returnForms.push_back(akForm);
                }
            }
        }
    }
    else {
        for (int i = 0; i < refsSize; i++) {
            if (refs[i]) {
                RE::TESForm* akForm = refs[i]->GetBaseObject();
                if (akForm) {
                    if (std::find(returnForms.begin(), returnForms.end(), akForm) == returnForms.end()) {
                        returnForms.push_back(akForm);
                    }
                }
            }
        }
    }

    return returnForms;
}

bool Has_DLL(RE::StaticFunctionTag*) {
    return true;
}

std::vector<int> Get_Version(RE::StaticFunctionTag*) {
    std::vector<int> version {1, 1, 3};
    return version;
}

bool Has_Version(RE::StaticFunctionTag*, int major, int minor, int patch, std::string mode) {
    std::vector<int> version = Get_Version(nullptr);

    int cVersion = ((major * 100) + (minor * 10) + patch);
    int fVersion = ((version[0] * 100) + (version[1] * 10) + version[2]);

    if (mode == "!=") {
        return (fVersion != cVersion);
    }
    else if (mode == "<") {
        return (fVersion < cVersion);
    }
    else if (mode == ">") {
        return (fVersion > cVersion);
    }
    else if (mode == "<=") {
        return (fVersion <= cVersion);
    }
    else if (mode == ">=") {
        return (fVersion >= cVersion);
    }

    return (fVersion == cVersion);
}

float Microseconds(RE::StaticFunctionTag*) {
    std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(end - pluginStartTimePoint);
    return float(duration.count());
}

float Milliseconds(RE::StaticFunctionTag*) {
    std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(end - pluginStartTimePoint);
    return float(duration.count());
}

float Seconds(RE::StaticFunctionTag*) {
    std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::seconds>(end - pluginStartTimePoint);
    return float(duration.count());
}

bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    logger::info("Binding Papyrus Functions");

    //functions 
    vm->RegisterFunction("All", "SkyPal_References", All);
    vm->RegisterFunction("All_Filter_Bases", "SkyPal_References", All_Filter_Bases);
    vm->RegisterFunction("All_Filter_Bases_Form_List", "SkyPal_References", All_Filter_Bases_Form_List);
    vm->RegisterFunction("Grid", "SkyPal_References", Grid);
    vm->RegisterFunction("Grid_Filter_Bases", "SkyPal_References", Grid_Filter_Bases);
    vm->RegisterFunction("Grid_Filter_Bases_Form_List", "SkyPal_References", Grid_Filter_Bases_Form_List);
    vm->RegisterFunction("Count_Disabled", "SkyPal_References", Count_Disabled);
    vm->RegisterFunction("Count_Enabled", "SkyPal_References", Count_Enabled);
    vm->RegisterFunction("Disable", "SkyPal_References", Disable);
    vm->RegisterFunction("Enable", "SkyPal_References", Enable);
    vm->RegisterFunction("Filter_Bases", "SkyPal_References", Filter_Bases);
    vm->RegisterFunction("Filter_Base_Form_Types", "SkyPal_References", Filter_Base_Form_Types);
    vm->RegisterFunction("Filter_Bases_Form_List", "SkyPal_References", Filter_Bases_Form_List);
    vm->RegisterFunction("Filter_Collision_Layer_Types", "SkyPal_References", Filter_Collision_Layer_Types);
    vm->RegisterFunction("Change_Collision_Layer_Type", "SkyPal_References", Change_Collision_Layer_Type);
    vm->RegisterFunction("Filter_Deleted", "SkyPal_References", Filter_Deleted);
    vm->RegisterFunction("Filter_Distance", "SkyPal_References", Filter_Distance);
    vm->RegisterFunction("Filter_Enabled", "SkyPal_References", Filter_Enabled);
    vm->RegisterFunction("Filter_Form_Types", "SkyPal_References", Filter_Form_Types);
    vm->RegisterFunction("Filter_Keywords", "SkyPal_References", Filter_Keywords);
    vm->RegisterFunction("Filter_Owners", "SkyPal_References", Filter_Owners);
    vm->RegisterFunction("Filter_Potential_Thieves", "SkyPal_References", Filter_Potential_Thieves);
    vm->RegisterFunction("Sort_Distance", "SkyPal_References", Sort_Distance);

    vm->RegisterFunction("From_References", "SkyPal_Bases", From_References);

    vm->RegisterFunction("Has_DLL", "SkyPal", Has_DLL);
    vm->RegisterFunction("Has_Version", "SkyPal", Has_Version);
    vm->RegisterFunction("Microseconds", "SkyPal", Microseconds);
    vm->RegisterFunction("Milliseconds", "SkyPal", Milliseconds);
    vm->RegisterFunction("Seconds", "SkyPal", Seconds);

    vm->RegisterFunction("Get_Version", "skypal_refs_ng", Get_Version);
    vm->RegisterFunction("Filter_WorldSpace", "skypal_refs_ng", Filter_WorldSpace);
    vm->RegisterFunction("Filter_3dLoaded", "skypal_refs_ng", Filter_3dLoaded);
    vm->RegisterFunction("Filter_OffLimits", "skypal_refs_ng", Filter_OffLimits);
    vm->RegisterFunction("Filter_InventoryObjects", "skypal_refs_ng", Filter_InventoryObjects);
    vm->RegisterFunction("Filter_PlayableObjects", "skypal_refs_ng", Filter_PlayableObjects);
    vm->RegisterFunction("Filter_QuestObjects", "skypal_refs_ng", Filter_QuestObjects);
    vm->RegisterFunction("CountNumberOfKeywordsRefHas", "skypal_refs_ng", CountNumberOfKeywordsRefHas);
    vm->RegisterFunction("refHasAtLeastOneKeyword", "skypal_refs_ng", refHasAtLeastOneKeyword);
    vm->RegisterFunction("filter_keywordsOnRef", "skypal_refs_ng", filter_keywordsOnRef);
    vm->RegisterFunction("ActorIsOwnerOfRef", "skypal_refs_ng", ActorIsOwnerOfRef);
    vm->RegisterFunction("CountOwnersForRef", "skypal_refs_ng", CountOwnersForRef);
    vm->RegisterFunction("RefHasAtLeastOneOwner", "skypal_refs_ng", RefHasAtLeastOneOwner);
    vm->RegisterFunction("filter_OwnersOnRef", "skypal_refs_ng", filter_OwnersOnRef);
    vm->RegisterFunction("ActorIsPotentialThiefOfRef", "skypal_refs_ng", ActorIsPotentialThiefOfRef);
    vm->RegisterFunction("CountPotentialThievesForRef", "skypal_refs_ng", CountPotentialThievesForRef);
    vm->RegisterFunction("RefHasAtLeastOnePotentialThief", "skypal_refs_ng", RefHasAtLeastOnePotentialThief);
    vm->RegisterFunction("filter_PotentialThievesOnRef", "skypal_refs_ng", filter_PotentialThievesOnRef);

    return true;
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);

    SetupLog();
    SKSE::GetPapyrusInterface()->Register(BindPapyrusFunctions);
    pluginStartTimePoint = std::chrono::high_resolution_clock::now();

    // Once all plugins and mods are loaded, then the ~ console is ready and can
    // be printed to
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message *message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded)
            RE::ConsoleLog::GetSingleton()->Print("Skypal NG Installed");
    });
    return true;
}