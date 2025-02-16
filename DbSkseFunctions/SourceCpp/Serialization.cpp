#include "Serialization.h"
#include "GeneralFunctions.h"

namespace logger = SKSE::log;

namespace serialize {

    std::vector<RE::TESObjectREFR*> LoadObjectRefVector(uint32_t record, SKSE::SerializationInterface* ssi) {
        std::string sRecord = gfuncs::uint32_to_string(record);

        std::vector<RE::TESObjectREFR*> v;
        std::size_t size;

        if (!ssi->ReadRecordData(size)) {
            logger::error("{}: record[{}] Failed to load size of v", __func__, sRecord);
            return v;
        }

        logger::trace("{}: record[{}] size[{}]", __func__, sRecord, size);

        for (std::size_t i = 0; i < size; ++i) {
            RE::FormID formID;
            if (!ssi->ReadRecordData(formID)) {
                logger::error("{}: record[{}] i[{}] filed to load formID", __func__, sRecord, i);
                return v;
            }

            if (formID == 0) {
                logger::trace("{}: i[{}] formId is 0", __func__, i);
                continue;
            }
            if (!ssi->ResolveFormID(formID, formID)) {
                logger::warn("{}: record[{}] i[{}] warning, failed to resolve formID[{:x}]", __func__, sRecord, i, formID);
                continue;
            }

            RE::TESObjectREFR* ref = RE::TESForm::LookupByID<RE::TESObjectREFR>(formID);
            if (gfuncs::IsFormValid(ref)) {
                v.push_back(ref);
            }
        }
        return v;
    }

    bool SaveObjectRefVector(std::vector<RE::TESObjectREFR*>& v, uint32_t record, SKSE::SerializationInterface* ssi) {
        std::string sRecord = gfuncs::uint32_to_string(record);

        if (!ssi->OpenRecord(record, 1)) {
            logger::error("{}: Failed to open record[{}]", __func__, sRecord);
            return false;
        }

        const std::size_t size = v.size();
        logger::trace("{}: record[{}] size[{}]", __func__, sRecord, size);

        if (!ssi->WriteRecordData(size)) {
            logger::error("{}: record[{}] Failed to write size of v", __func__, sRecord);
            return false;
        }

        for (std::size_t i = 0; i < size; i++) {
            if (gfuncs::IsFormValid(v[i])) {
                RE::FormID formID = v[i]->GetFormID();
                if (!ssi->WriteRecordData(formID)) {
                    logger::error("{}: record[{}] Failed to write data for formID[{:x}]", __func__, sRecord, formID);
                    return false;
                }
            }
            else {
                RE::FormID noformID = 0;
                if (!ssi->WriteRecordData(noformID)) {
                    logger::error("{}: record[{}] Failed to write data for noformID", __func__, sRecord);
                    return false;
                }
            }
        }

        return true;
    }

    std::vector<RE::TESForm*> LoadFormVector(uint32_t record, SKSE::SerializationInterface* ssi) {
        std::string sRecord = gfuncs::uint32_to_string(record);

        std::vector<RE::TESForm*> v;
        std::size_t size;

        if (!ssi->ReadRecordData(size)) {
            logger::error("{}: record[{}] Failed to load size of v", __func__, sRecord);
            return v;
        }

        logger::trace("{}: record[{}] size[{}]", __func__, sRecord, size);

        for (std::size_t i = 0; i < size; ++i) {
            RE::FormID formID;
            if (!ssi->ReadRecordData(formID)) {
                logger::error("{}: record[{}] i[{}] Failed to load formID,", __func__, sRecord, i);
                return v;
            }

            if (!ssi->ResolveFormID(formID, formID)) {
                logger::warn("{}: record[{}] i[{}] warning, failed to resolve formID[{:x}], ", __func__, sRecord, i, formID);
                continue;
            }

            RE::TESForm* akForm = RE::TESForm::LookupByID(formID);
            if (gfuncs::IsFormValid(akForm)) {
                v.push_back(akForm);
            }
        }
        return v;
    }

    bool SaveFormVector(std::vector<RE::TESForm*>& v, uint32_t record, SKSE::SerializationInterface* ssi) {
        std::string sRecord = gfuncs::uint32_to_string(record);

        if (!ssi->OpenRecord(record, 1)) {
            logger::error("{}: Failed to open record[{}]", __func__, sRecord);
            return false;
        }

        const std::size_t size = v.size();
        logger::trace("{}: record[{}] size[{}]", __func__, sRecord, size);

        if (!ssi->WriteRecordData(size)) {
            logger::error("{}: record[{}] Failed to write size of v", __func__, sRecord);
            return false;
        }

        for (std::size_t i = 0; i < size; i++) {

            if (gfuncs::IsFormValid(v[i])) {
                RE::FormID formID = v[i]->GetFormID();
                if (!ssi->WriteRecordData(formID)) {
                    logger::error("{}: record[{}] Failed to write data for formID[{:x}]", __func__, sRecord, formID);
                    return false;
                }
            }
            else {
                RE::FormID noformID = 0;
                if (!ssi->WriteRecordData(noformID)) {
                    logger::error("{}: record[{}] Failed to write data for noformID", __func__, sRecord);
                    return false;
                }
            }
        }

        return true;
    }
}