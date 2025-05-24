#pragma once

#include "GeneralFunctions.h"

namespace serialize {
    bool SetSerializing(bool serializing);

    RE::TESForm* LoadForm(SKSE::SerializationInterface* ssi);
    
    //exampe: RE::TESWeather* weather = serialize::LoadForm<RE::TESWeather>(ssi);
    template <typename T>
    T* LoadForm(SKSE::SerializationInterface* ssi) {
        RE::FormID formID;
        if (!ssi->ReadRecordData(formID)) {
            //logger::error("Failed to load formID!");
            return nullptr;
        }

        if (!ssi->ResolveFormID(formID, formID)) {
            logger::warn("Warning: failed to resolve formID[{:x}]", formID);
        }

        RE::TESForm* baseForm = RE::TESForm::LookupByID(formID);

        if (!gfuncs::IsFormValid(baseForm, false)) {
            //logger::error("Form is not valid");
            return nullptr;
        }

        T* castedForm = skyrim_cast<T*>(baseForm);
        if (!castedForm) {
            logger::error("Failed to cast formID [{:x}] to the desired type", formID);
            return nullptr;
        }

        return castedForm;
    }

    bool SaveForm(RE::TESForm* akForm, uint32_t record, SKSE::SerializationInterface* ssi, bool openRecord = true);

    std::vector<RE::TESForm*> LoadFormVector(uint32_t record, SKSE::SerializationInterface* ssi);
    
    //example: std::vector<RE::TESWeather*> weathers = serialize::LoadFormVector<RE::TESWeather>(type, ssi);
    template <typename T>
    std::vector<T*> LoadFormVector(uint32_t record, SKSE::SerializationInterface* ssi) {
        std::string sRecord = gfuncs::uint32_to_string(record);

        std::vector<T*> v;
        std::size_t size;

        if (!ssi->ReadRecordData(size)) {
            logger::error("record[{}] Failed to load size of v", sRecord);
            return v;
        }

        logger::trace("record[{}] size[{}]", sRecord, size);

        for (std::size_t i = 0; i < size; ++i) {
            T* castedForm = LoadForm<T>(ssi);
            if (castedForm) {
                v.push_back(castedForm);
            }
            else {
                logger::warn("loaded form for record[{}] index[{}] is null.", sRecord, i);
            }
        }
        return v;
    }

    template <typename T>
    bool SaveFormVector(std::vector<T*>& v, uint32_t record, SKSE::SerializationInterface* ssi, bool openRecord = true) {
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
            RE::TESForm* form = v[i];
            if (gfuncs::IsFormValid(form)) {
                RE::FormID formID = form->GetFormID();
                if (!ssi->WriteRecordData(formID)) {
                    logger::error("record[{}] index[{}] Failed to write data for formID[{:x}]", sRecord, i, formID);
                    return false;
                }
            }
            else {
                RE::FormID noformID = 0;
                if (!ssi->WriteRecordData(noformID)) {
                    logger::error("record[{}] index[{}] Failed to write data for noformID", sRecord, i);
                    return false;
                }
            }
        }

        return true;
    }

    bool LoadFormIDVector(std::vector<RE::FormID>& arr, uint32_t record, SKSE::SerializationInterface* ssi);

    bool SaveFormIDVector(std::vector<RE::FormID>& arr, uint32_t record, SKSE::SerializationInterface* ssi, bool openRecord = true);

    bool LoadHandlesVector(std::vector<RE::VMHandle>& arr, uint32_t record, SKSE::SerializationInterface* ssi);

    bool SaveHandlesVector(std::vector<RE::VMHandle>& arr, uint32_t record, SKSE::SerializationInterface* ssi, bool openRecord = true);

    bool LoadFormHandlesMap(std::map<RE::TESForm*, std::vector<RE::VMHandle>>& akMap, uint32_t record, SKSE::SerializationInterface* ssi);

    bool SaveFormHandlesMap(std::map<RE::TESForm*, std::vector<RE::VMHandle>>& akMap, uint32_t record, SKSE::SerializationInterface* ssi, bool openRecord = true);

    std::vector<RE::TESObjectREFR*> LoadObjectRefVector(uint32_t record, SKSE::SerializationInterface* ssi);

    bool SaveObjectRefVector(std::vector<RE::TESObjectREFR*>& v, uint32_t record, SKSE::SerializationInterface* ssi, bool openRecord = true);
}