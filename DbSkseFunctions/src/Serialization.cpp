#include "Serialization.h"
#include "GeneralFunctions.h"

namespace serialize {
    bool bIsSerializing = false;
    std::mutex mutex;

    bool SetSerializing(bool set) {
        bool bReturn = false;
        mutex.lock();
        if (set) {
            if (!bIsSerializing) { //can only set true if not already serializing
                bIsSerializing = true; 
                bReturn = true;
            }
        }
        else {
            bIsSerializing = false;
            bReturn = true;
        }
        mutex.unlock();
        return bReturn;
    }

    RE::TESForm* LoadForm(SKSE::SerializationInterface* ssi) {
        RE::FormID formID;
        if (!ssi->ReadRecordData(formID)) {
            //logger::error("Failed to load formID!");
            return nullptr;
        }

        if (!ssi->ResolveFormID(formID, formID)) {
            logger::warn("warning, failed to resolve formID[{:x}]", formID);
        }

        RE::TESForm* akForm = RE::TESForm::LookupByID(formID);

        if (!gfuncs::IsFormValid(akForm, false)) {
            //logger::error("akForm isn't valid");
            return nullptr;
        }

        return akForm;
    }

    bool SaveForm(RE::TESForm* akForm, uint32_t record, SKSE::SerializationInterface* ssi, bool openRecord) {
        if (openRecord) {
            if (!ssi->OpenRecord(record, 1)) {
                logger::error("Failed to open record[{}]", gfuncs::uint32_to_string(record));
                return false;
            }
        }

        if (gfuncs::IsFormValid(akForm)) {
            RE::FormID formID = akForm->GetFormID();
            if (!ssi->WriteRecordData(formID)) {
                logger::error("record[{}] Failed to write data for formID[{:x}]", gfuncs::uint32_to_string(record), formID);
                return false;
            }
        }
        else {
            RE::FormID noformID = 0;
            if (!ssi->WriteRecordData(noformID)) {
                logger::error("record[{}] Failed to write data for noformID", gfuncs::uint32_to_string(record));
                return false;
            }
        }
        return true;
    } 

    std::vector<RE::TESForm*> LoadFormVector(uint32_t record, SKSE::SerializationInterface* ssi) {
        std::string sRecord = gfuncs::uint32_to_string(record);

        std::vector<RE::TESForm*> v;
        std::size_t size;

        if (!ssi->ReadRecordData(size)) {
            logger::error("record[{}] Failed to load size of v", sRecord);
            return v;
        }

        logger::trace("record[{}] size[{}]", sRecord, size);

        for (std::size_t i = 0; i < size; ++i) {
            RE::TESForm* akForm = LoadForm(ssi);
            if (akForm) {
                v.push_back(akForm);
            }
            else {
                logger::warn("loaded form for index[{}] record[{}] is null.", i, sRecord);
            }
        }
        return v;
    } 

    bool LoadFormIDVector(std::vector<RE::FormID>& arr, uint32_t record, SKSE::SerializationInterface* ssi) {
        arr.clear();
        std::size_t size;

        if (!ssi->ReadRecordData(size)) {
            logger::error("record: [{}] Failed to load size of arr!", record);
            return false;
        }

        logger::debug("load arr size = {}", size);

        for (std::size_t i = 0; i < size; ++i) {
            RE::FormID formID;
            if (!ssi->ReadRecordData(formID)) {
                logger::error("{}: Failed to load formID!", i);
                return false;
            }

            if (!ssi->ResolveFormID(formID, formID)) {
                logger::warn("{}: failed to resolve formID[{:x}]", i, formID);
                continue;
            }

            arr.push_back(formID);
        }
        return true;
    }

    bool SaveFormIDVector(std::vector<RE::FormID>& arr, uint32_t record, SKSE::SerializationInterface* ssi, bool openRecord) {
        if (openRecord) {
            if (!ssi->OpenRecord(record, 1)) {
                logger::error("Failed to open record[{}]", gfuncs::uint32_to_string(record));
                return false;
            }
        }

        const std::size_t size = arr.size();

        if (!ssi->WriteRecordData(size)) {
            logger::error("record[{}] Failed to write size of arr!", record);
            return false;
        }

        for (std::size_t i = 0; i < size; i++) {
            RE::FormID& formID = arr[i];

            if (formID) {
                if (!ssi->WriteRecordData(formID)) {
                    logger::error("record[{}] Failed to write data for formID[{}]", record, formID);
                    return false;
                }
            }
            else {
                RE::FormID noFormID = -1;
                if (!ssi->WriteRecordData(noFormID)) {
                    logger::error("record[{}] Failed to write data for noFormID", record);
                    return false;
                }
            }
        }

        return true;
    }

    bool LoadHandlesVector(std::vector<RE::VMHandle>& arr, uint32_t record, SKSE::SerializationInterface* ssi) {
        arr.clear();
        std::size_t size;
        if (!ssi->ReadRecordData(size)) {
            logger::error("record: [{}] Failed to load size of arr!", record);
            return false;
        }

        logger::trace("load arr size = {}", size);

        for (std::size_t i = 0; i < size; ++i) {
            RE::VMHandle handle;
            if (!ssi->ReadRecordData(handle)) {
                logger::error("{}: Failed to load handle!", i);
                return false;
            }

            if (!ssi->ResolveHandle(handle, handle)) {
                logger::warn("{}: warning, failed to resolve handle[{}]", i, handle);
                continue;
            }

            arr.push_back(handle);
        }
        return true;
    }

    bool SaveHandlesVector(std::vector<RE::VMHandle>& arr, uint32_t record, SKSE::SerializationInterface* ssi, bool openRecord) {
        if (openRecord) {
            if (!ssi->OpenRecord(record, 1)) {
                logger::error("Failed to open record[{}]", gfuncs::uint32_to_string(record));
                return false;
            }
        }
        
        const std::size_t size = arr.size();

        if (!ssi->WriteRecordData(size)) {
            logger::error("record[{}] Failed to write size of arr!", record);
            return false;
        }

        for (std::size_t i = 0; i < size; i++) {
            RE::VMHandle& handle = arr[i];

            if (handle) {
                if (!ssi->WriteRecordData(handle)) {
                    logger::error("record[{}] Failed to write data for handle[{}]", record, handle);
                    return false;
                }
            }
            else {
                RE::VMHandle noHandle = -1;
                if (!ssi->WriteRecordData(noHandle)) {
                    logger::error("record[{}] Failed to write data for noHandle", record);
                    return false;
                }
            }
        }

        return true;
    }

    bool LoadFormHandlesMap(std::map<RE::TESForm*, std::vector<RE::VMHandle>>& akMap, uint32_t record, SKSE::SerializationInterface* ssi) {
        akMap.clear();

        std::size_t size;

        if (!ssi->ReadRecordData(size)) {
            logger::error("record[{}] Failed to load size of akMap!", record);
            return false;
        }

        logger::debug("load akMap size = {}", size);

        for (std::size_t i = 0; i < size; ++i) { //size = number of pairs in map 

            RE::FormID formID;

            if (!ssi->ReadRecordData(formID)) {
                logger::error("{}: Failed to load formID!", i);
                return false;
            }

            bool formIdResolved = ssi->ResolveFormID(formID, formID);

            if (!formIdResolved) {
                logger::warn("{}: Failed to resolve formID {:x}", i, formID);
                return false;
            }

            //logger::trace("{}: formID[{:x}] loaded and resolved", i, formID);

            RE::TESForm* akForm;
            if (formIdResolved) {
                akForm = RE::TESForm::LookupByID<RE::TESForm>(formID);
                if (!gfuncs::IsFormValid(akForm)) {
                    logger::error("{}: error, failed to load akForm!", i);
                    return false;
                }
                else {
                    logger::debug("{}: akForm[{}] loaded", i, formID);
                }
            }

            std::size_t handlesSize;

            if (!ssi->ReadRecordData(handlesSize)) {
                logger::error("{}: Failed to load handlesSize!", i);
                return false;
            }

            logger::debug("{}: handlesSize loaded. Size[{}]", i, handlesSize);

            std::vector<RE::VMHandle> handles;

            for (std::size_t ib = 0; ib < handlesSize; ib++) {
                RE::VMHandle handle;

                if (!ssi->ReadRecordData(handle)) {
                    logger::error("{}: Failed to load handle", ib);
                    return false;
                }

                if (!ssi->ResolveHandle(handle, handle)) {
                    logger::warn("{}: Failed to resolve handle {}", ib, handle);
                    return false;
                }
                else {
                    handles.push_back(handle);
                }
            }

            if (handles.size() > 0 && akForm != nullptr) {
                akMap.insert(std::pair<RE::TESForm*, std::vector<RE::VMHandle>>(akForm, handles));
                logger::debug("{}: record[{}] akForm[{}] formID[{:x}] loaded", i, record, gfuncs::GetFormName(akForm), formID);
            }
        }
        return true;
    }

    bool SaveFormHandlesMap(std::map<RE::TESForm*, std::vector<RE::VMHandle>>& akMap, uint32_t record, SKSE::SerializationInterface* ssi, bool openRecord) {
        if (openRecord) {
            if (!ssi->OpenRecord(record, 1)) {
                logger::error("Failed to open record[{}]", gfuncs::uint32_to_string(record));
                return false;
            }
        }
        
        const std::size_t  size = akMap.size();

        if (!ssi->WriteRecordData(size)) {
            logger::error("Failed to write size of akMap!");
            return false;
        }
        else {
            std::map<RE::TESForm*, std::vector<RE::VMHandle>>::iterator it;

            int ic = 0;
            for (it = akMap.begin(); it != akMap.end() && ic < akMap.size(); it++, ic++) {

                RE::FormID formID = -1;
                if (it->first) {
                    formID = it->first->GetFormID();
                    logger::trace("saving handles for ref[{}] formId[{:x}]", gfuncs::GetFormName(it->first), formID);
                }

                if (!ssi->WriteRecordData(formID)) {
                    logger::error("Failed to write formID[{:x}]", formID);
                    return false;
                }

                logger::trace("formID[{:x}] written successfully", formID);

                const std::size_t handlesSize = it->second.size();

                if (!ssi->WriteRecordData(handlesSize)) {
                    logger::error("failed to write it.second handlesSize");
                    return false;
                }

                for (const RE::VMHandle& handle : it->second) {
                    if (handle) {
                        if (!ssi->WriteRecordData(handle)) {
                            logger::error("Failed to write data for handle[{}]", handle);
                            return false;
                        }
                    }
                    else {
                        RE::VMHandle noHandle;
                        if (!ssi->WriteRecordData(noHandle)) {
                            logger::error("Failed to write data for noHandle", handle);
                            return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    std::vector<RE::TESObjectREFR*> LoadObjectRefVector(uint32_t record, SKSE::SerializationInterface* ssi) {
        std::string sRecord = gfuncs::uint32_to_string(record);

        std::vector<RE::TESObjectREFR*> v;
        std::size_t size;

        if (!ssi->ReadRecordData(size)) {
            logger::error("record[{}] Failed to load size of v", sRecord);
            return v;
        }

        logger::trace("record[{}] size[{}]", sRecord, size);

        for (std::size_t i = 0; i < size; ++i) {
            RE::FormID formID;
            if (!ssi->ReadRecordData(formID)) {
                logger::error("record[{}] i[{}] filed to load formID", sRecord, i);
                return v;
            }

            if (formID == 0) {
                logger::trace("i[{}] formId is 0", i);
                continue;
            }
            if (!ssi->ResolveFormID(formID, formID)) {
                logger::warn("record[{}] i[{}] warning, failed to resolve formID[{:x}]", sRecord, i, formID);
                continue;
            }

            RE::TESObjectREFR* ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(formID);
            if (gfuncs::IsFormValid(ref)) {
                v.push_back(ref);
            }
        }
        return v;
    }

    bool SaveObjectRefVector(std::vector<RE::TESObjectREFR*>& v, uint32_t record, SKSE::SerializationInterface* ssi, bool openRecord) {
        std::string sRecord = gfuncs::uint32_to_string(record);
        
        if (openRecord) {
            if (!ssi->OpenRecord(record, 1)) {
                logger::error("Failed to open record[{}]", sRecord);
                return false;
            }
        }

        const std::size_t size = v.size();
        logger::trace("record[{}] size[{}]", sRecord, size);

        if (!ssi->WriteRecordData(size)) {
            logger::error("record[{}] Failed to write size of v", sRecord);
            return false;
        }

        for (std::size_t i = 0; i < size; i++) {
            if (gfuncs::IsFormValid(v[i])) {
                RE::FormID formID = v[i]->GetFormID();
                if (!ssi->WriteRecordData(formID)) {
                    logger::error("record[{}] Failed to write data for formID[{:x}]", sRecord, formID);
                    return false;
                }
            }
            else {
                RE::FormID noformID = 0;
                if (!ssi->WriteRecordData(noformID)) {
                    logger::error("record[{}] Failed to write data for noformID", sRecord);
                    return false;
                }
            }
        }

        return true;
    }

}