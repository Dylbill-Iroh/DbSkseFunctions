#pragma once

namespace serialize {
    std::vector<RE::TESObjectREFR*> LoadObjectRefVector(uint32_t record, SKSE::SerializationInterface* ssi);

    bool SaveObjectRefVector(std::vector<RE::TESObjectREFR*>& v, uint32_t record, SKSE::SerializationInterface* ssi);

    std::vector<RE::TESForm*> LoadFormVector(uint32_t record, SKSE::SerializationInterface* ssi);

    bool SaveFormVector(std::vector<RE::TESForm*>& v, uint32_t record, SKSE::SerializationInterface* ssi);
}