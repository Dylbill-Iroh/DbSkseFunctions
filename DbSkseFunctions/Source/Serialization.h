#pragma once

namespace serialize {
    RE::TESForm* LoadForm(SKSE::SerializationInterface* a_intfc);

    bool LoadFormIDVector(std::vector<RE::FormID>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc);

    bool SaveFormIDVector(std::vector<RE::FormID>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc);

    bool LoadHandlesVector(std::vector<RE::VMHandle>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc);

    bool SaveHandlesVector(std::vector<RE::VMHandle>& arr, uint32_t record, SKSE::SerializationInterface* a_intfc);

    bool LoadFormHandlesMap(std::map<RE::TESForm*, std::vector<RE::VMHandle>>& akMap, uint32_t record, SKSE::SerializationInterface* a_intfc);

    bool SaveFormHandlesMap(std::map<RE::TESForm*, std::vector<RE::VMHandle>>& akMap, uint32_t record, SKSE::SerializationInterface* a_intfc);

    std::vector<RE::TESObjectREFR*> LoadObjectRefVector(uint32_t record, SKSE::SerializationInterface* ssi);

    bool SaveObjectRefVector(std::vector<RE::TESObjectREFR*>& v, uint32_t record, SKSE::SerializationInterface* ssi);

    std::vector<RE::TESForm*> LoadFormVector(uint32_t record, SKSE::SerializationInterface* ssi);

    bool SaveFormVector(std::vector<RE::TESForm*>& v, uint32_t record, SKSE::SerializationInterface* ssi);
}